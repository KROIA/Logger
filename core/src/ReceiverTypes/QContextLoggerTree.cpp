#include "ReceiverTypes/QContextLoggerTree.h"

#ifdef LOGGER_QT

namespace Log
{
	namespace Receiver
	{
		QContextLoggerTree::QContextLoggerTree(QTreeWidget* parent)
			: QWidget(parent)
			, m_treeWidget(parent)
		{
			m_updateTimer.setInterval(100);
			connect(&m_updateTimer, &QTimer::timeout, this, &QContextLoggerTree::onUpdateTimer);
			m_updateTimer.start();

			m_treeWidget->setColumnCount(3);
			QStringList headerLables;
			for (int i = 0; i < (int)HeaderPos::__count; ++i)
			{
				headerLables << getHeaderName((HeaderPos)i);
				m_treeWidget->setColumnWidth(i, getHeaderWidth((HeaderPos)i));
			}
			m_treeWidget->setHeaderLabels(headerLables);
		}

		QContextLoggerTree::~QContextLoggerTree()
		{

		}


		const QString& QContextLoggerTree::getHeaderName(HeaderPos pos) const
		{
			switch (pos)
			{
			case HeaderPos::contextName: { static QString s = "Context"; return s; }
			case HeaderPos::timestamp: { static QString s = "Timestamp"; return s; }
			case HeaderPos::message: { static QString s = "Message"; return s; }
			}
			static QString s;
			return s;
		}
		unsigned int QContextLoggerTree::getHeaderWidth(HeaderPos pos) const
		{
			switch (pos)
			{
			case HeaderPos::contextName: { return 200; }
			case HeaderPos::timestamp: { return 150; }
			case HeaderPos::message: { return 500; }
			}
			return 0;
		}

		void QContextLoggerTree::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			Logger::ContextLogger *contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
			auto& it = m_msgItems.find(contextLogger);
			if (it != m_msgItems.end())
				return;

			/*onContextCreate(*contextLogger);
			return;*/

			TreeData *treeData = new TreeData();
			treeData->attachLogger(logger);
			treeData->logger = contextLogger;
			QTreeWidgetItem* newItem = new QTreeWidgetItem(m_treeWidget);
			QTreeWidgetItem* newMessageRoot = new QTreeWidgetItem(newItem);
			treeData->childRoot = newItem;
			treeData->thisMessagesRoot = newMessageRoot;

			newItem->setData((int)HeaderPos::contextName, Qt::DisplayRole, logger.getName().c_str());
			newItem->setData((int)HeaderPos::timestamp, Qt::DisplayRole, logger.getCreationDateTime().toString().c_str());
			QColor contextColor = logger.getColor().toQColor();
			newItem->setBackgroundColor((int)HeaderPos::contextName, contextColor);

			newMessageRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
			newMessageRoot->setBackgroundColor((int)HeaderPos::contextName, contextColor);
			const std::vector<Message>& messages = logger.getMessages();
			m_msgItems[&logger] = treeData;
			for (size_t i = 0; i < messages.size(); ++i)
			{
				onNewMessage(messages[i]);
			}

			const std::vector<Logger::ContextLogger*>& childs = contextLogger->getChilds();
			for (size_t i = 0; i < childs.size(); ++i)
			{
				onContextCreate(*childs[i]);
			}


			unsigned int dummy;
			updateMessageCount(*contextLogger, dummy);
		}
		void QContextLoggerTree::onUnsubscribed(Logger::AbstractLogger& logger)
		{

		}
		void QContextLoggerTree::onContextCreate(Logger::ContextLogger& newContext)
		{
			Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&newContext);
			const Logger::ContextLogger* parent = contextLogger->getParent();
			auto& parentIt = m_msgItems.find(parent);

			parentIt = m_msgItems.find(parent);
			if (parentIt == m_msgItems.end())
			{
				return;
			}

			if (m_msgItems.find(contextLogger) != m_msgItems.end())
				return;

			QTreeWidgetItem* newItem = new QTreeWidgetItem(parentIt->second->childRoot);
			QTreeWidgetItem* newMessageRoot = new QTreeWidgetItem(newItem);
			TreeData *treeData = new TreeData();
			treeData->attachLogger(newContext);
			treeData->logger = contextLogger;
			treeData->childRoot = newItem;
			treeData->thisMessagesRoot = newMessageRoot;

			newItem->setData((int)HeaderPos::contextName, Qt::DisplayRole, contextLogger->getName().c_str());
			newItem->setData((int)HeaderPos::timestamp, Qt::DisplayRole, contextLogger->getCreationDateTime().toString().c_str());
			QColor contextColor = contextLogger->getColor().toQColor();
			newItem->setBackgroundColor((int)HeaderPos::contextName, contextColor);

			newMessageRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
			newMessageRoot->setBackgroundColor((int)HeaderPos::contextName, contextColor);

			const std::vector<Message>& messages = contextLogger->getMessages();
			m_msgItems[contextLogger] = treeData;
			for (size_t i = 0; i < messages.size(); ++i)
			{
				onNewMessage(messages[i]);
			}
			unsigned int dummy;
			updateMessageCount(*contextLogger, dummy);
		}
		void QContextLoggerTree::onNewMessage(const Message& m)
		{
			Logger::ContextLogger* logger = dynamic_cast<Logger::ContextLogger*>(m.getContext());
			auto& it = m_msgItems.find(logger);
			if (it == m_msgItems.end())
			{
				if (logger)
				{
					onContextCreate(*logger);
				}
			}

			it = m_msgItems.find(logger);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;

			QTreeWidgetItem* line = new QTreeWidgetItem(treeData->thisMessagesRoot);
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString().c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, m.getText().c_str());

			line->setIcon((int)HeaderPos::contextName, getIcon(m.getLevel()));
			line->setBackgroundColor((int)HeaderPos::message, m.getColor().toQColor());
			if (m.getContext())
			{
				QColor contextColor = m.getContext()->getColor().toQColor();
				line->setBackgroundColor((int)HeaderPos::contextName, contextColor);
			}
			

			treeData->msgItems.push_back(line);
			treeData->thisMessagesRoot->addChild(line);

			unsigned int dummy;
			updateMessageCount(*logger, dummy);
		}
		void QContextLoggerTree::onClear(Logger::AbstractLogger& logger)
		{
			auto& it = m_msgItems.find(&logger);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;

			for (size_t i = 0; i < treeData->msgItems.size(); ++i)
				delete treeData->msgItems[i];
			treeData->msgItems.clear();
			unsigned int dummy;
			updateMessageCount(*dynamic_cast<const Logger::ContextLogger*>(&logger), dummy);
		}
		void QContextLoggerTree::onContextDestroy(Logger::ContextLogger& loggerToDestroy)
		{
			auto& it = m_msgItems.find(&loggerToDestroy);
			if (it == m_msgItems.end())
				return;
			TreeData *treeData = it->second;

			delete treeData->childRoot;
			m_msgItems.erase(it);
		}
		void QContextLoggerTree::onDelete(Logger::AbstractLogger& loggerToDestroy)
		{
			onContextDestroy(*dynamic_cast<Logger::ContextLogger*>(&loggerToDestroy));
			detachLogger(loggerToDestroy);
		}
		void QContextLoggerTree::onUpdateTimer()
		{
			unsigned int dummy = 0;
			for (const Logger::AbstractLogger* logger : AbstractReceiver::getAttachedLoggers())
			{
				updateMessageCountRecursive(*dynamic_cast<const Logger::ContextLogger*>(logger), dummy);
			}
		}
		void QContextLoggerTree::updateMessageCount(const Logger::ContextLogger& logger, unsigned int& countOut)
		{
			auto& it = m_msgItems.find(&logger);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			countOut = logger.getMessages().size();
			QString text = "[" + QString::number(countOut) + "] Messages";
			treeData->thisMessagesRoot->setData((int)HeaderPos::message, Qt::DisplayRole, text);
		}
		void QContextLoggerTree::updateMessageCountRecursive(const Logger::ContextLogger& logger, unsigned int& countOut)
		{
			auto& it = m_msgItems.find(&logger);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			updateMessageCount(logger, countOut);

			const std::vector<Logger::ContextLogger*>& childs = logger.getChilds();
			for (size_t i = 0; i < childs.size(); ++i)
			{
				unsigned int childCount;
				updateMessageCountRecursive(*childs[i], childCount);
				countOut += childCount;
			}
			QString text = "[" + QString::number(countOut) + "] Messages";
			treeData->childRoot->setData((int)HeaderPos::message, Qt::DisplayRole, text);
		}


		void QContextLoggerTree::TreeData::onNewSubscribed(Logger::AbstractLogger& logger)
		{

		}
		void QContextLoggerTree::TreeData::onUnsubscribed(Logger::AbstractLogger& logger)
		{

		}
		void QContextLoggerTree::TreeData::onNewMessage(const Message& m)
		{
			QTreeWidgetItem* line = new QTreeWidgetItem(thisMessagesRoot);
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString().c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, m.getText().c_str());

			line->setIcon((int)HeaderPos::contextName, getIcon(m.getLevel()));
			line->setBackgroundColor((int)HeaderPos::message, m.getColor().toQColor());
			if (m.getContext())
			{
				QColor contextColor = m.getContext()->getColor().toQColor();
				line->setBackgroundColor((int)HeaderPos::contextName, contextColor);
			}

			msgItems.push_back(line);
			thisMessagesRoot->addChild(line);
		}
		void QContextLoggerTree::TreeData::onClear(Logger::AbstractLogger& logger)
		{
			for (size_t i = 0; i < msgItems.size(); ++i)
				delete msgItems[i];
			msgItems.clear();
		}
		void QContextLoggerTree::TreeData::onDelete(Logger::AbstractLogger& logger)
		{

		}
		void QContextLoggerTree::TreeData::onContextCreate(Logger::ContextLogger& logger)
		{

		}
		void QContextLoggerTree::TreeData::onContextDestroy(Logger::ContextLogger& logger)
		{

		}
	}
}
#endif