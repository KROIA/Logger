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
			m_timeFormat = DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond;
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
		void QContextLoggerTree::setDateTimeFormat(DateTime::Format format)
		{
			if(m_timeFormat == format)
				return;
			m_timeFormat = format;
			for (auto& context : m_msgItems)
			{
				context.second->updateDateTime();
			}
		}
		DateTime::Format QContextLoggerTree::getDateTimeFormat() const
		{
			return m_timeFormat;
		}

		void QContextLoggerTree::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			Logger::ContextLogger *contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
			auto& it = m_msgItems.find(contextLogger);
			if (it != m_msgItems.end())
				return;

			addContextRecursive(*contextLogger);
		}
		void QContextLoggerTree::onUnsubscribed(Logger::AbstractLogger& logger)
		{

		}
		void QContextLoggerTree::onContextCreate(Logger::ContextLogger& newContext)
		{
			const Logger::ContextLogger* parent = newContext.getParent();
			auto& parentIt = m_msgItems.find(parent);

			if (m_msgItems.find(&newContext) != m_msgItems.end())
				return;

			QTreeWidgetItem* newItem = nullptr;
			if (parentIt != m_msgItems.end())
				newItem = new QTreeWidgetItem(parentIt->second->childRoot);
			else
				newItem = new QTreeWidgetItem(m_treeWidget);

			QTreeWidgetItem* newMessageRoot = new QTreeWidgetItem(newItem);
			TreeData *treeData = new TreeData();
			treeData->parent = this;
			treeData->attachLogger(newContext);
			treeData->logger = &newContext;
			treeData->childRoot = newItem;
			treeData->thisMessagesRoot = newMessageRoot;

			newItem->setData((int)HeaderPos::contextName, Qt::DisplayRole, newContext.getName().c_str());
			newItem->setData((int)HeaderPos::timestamp, Qt::DisplayRole, newContext.getCreationDateTime().toString(m_timeFormat).c_str());
			QColor contextColor = newContext.getColor().toQColor();
			newItem->setBackgroundColor((int)HeaderPos::contextName, contextColor);
			newItem->setBackgroundColor((int)HeaderPos::timestamp, contextColor);
			newItem->setBackgroundColor((int)HeaderPos::message, contextColor);

			newMessageRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
			newMessageRoot->setBackgroundColor((int)HeaderPos::contextName, contextColor);
			newMessageRoot->setBackgroundColor((int)HeaderPos::timestamp, contextColor);
			newMessageRoot->setBackgroundColor((int)HeaderPos::message, contextColor);

			const std::vector<Message>& messages = newContext.getMessages();
			m_msgItems[&newContext] = treeData;
			for (size_t i = 0; i < messages.size(); ++i)
			{
				onNewMessage(messages[i]);
			}
			unsigned int dummy;
			updateMessageCount(newContext, dummy);
		}
		void QContextLoggerTree::addContextRecursive(Logger::ContextLogger& newContext)
		{
			onContextCreate(newContext);
			const std::vector<Logger::ContextLogger*>& childs = newContext.getChilds();
			for (size_t i = 0; i < childs.size(); ++i)
			{
				addContextRecursive(*childs[i]);
			}
		}
		void QContextLoggerTree::onNewMessage(const Message& m)
		{

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
			m_msgItems.erase(it);

			treeData->detachLogger(loggerToDestroy);
			
			//delete treeData->thisMessagesRoot;
			delete treeData;
			
		}
		void QContextLoggerTree::onDelete(Logger::AbstractLogger& loggerToDestroy)
		{
			onContextDestroy(*dynamic_cast<Logger::ContextLogger*>(&loggerToDestroy));
			detachLogger(loggerToDestroy);
		}
		void QContextLoggerTree::setContextVisibility(Logger::ContextLogger& logger, bool isVisible)
		{
			auto& it = m_msgItems.find(&logger);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->childRoot->setHidden(!isVisible);
		}
		bool QContextLoggerTree::getContextVisibility(Logger::ContextLogger& logger) const
		{
			auto& it = m_msgItems.find(&logger);
			if (it == m_msgItems.end())
				return false;
			TreeData* treeData = it->second;
			return !treeData->childRoot->isHidden();
		}
		void QContextLoggerTree::setLevelVisibility(Level level, bool isVisible)
		{
			if(level >= sizeof(m_levelVisibility) / sizeof(m_levelVisibility[0]))
				return;
			m_levelVisibility[level] = isVisible;
			for (auto& context : m_msgItems)
			{
				TreeData* treeData = context.second;
				for (size_t i = 0; i < treeData->msgItems.size(); ++i)
				{
					const Message& m = treeData->logger->getMessages()[i];
					if (m.getLevel() == level)
						treeData->msgItems[i]->setHidden(!isVisible);
				}
			}
		}
		bool QContextLoggerTree::getLevelVisibility(Level level) const
		{
			if (level >= sizeof(m_levelVisibility) / sizeof(m_levelVisibility[0]))
				return false;
			return m_levelVisibility[level];
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
				unsigned int childCount = 0;
				updateMessageCountRecursive(*childs[i], childCount);
				countOut += childCount;
			}
			QString text = "[" + QString::number(countOut) + "] Messages";
			treeData->childRoot->setData((int)HeaderPos::message, Qt::DisplayRole, text);
		}

		QContextLoggerTree::TreeData::~TreeData()
		{
			QTreeWidgetItem *childRoot_ = childRoot;
			childRoot = nullptr;
			thisMessagesRoot = nullptr;
			detachLogger(*logger);
			delete childRoot_;
		}
		void QContextLoggerTree::TreeData::updateDateTime()
		{
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, logger->getCreationDateTime().toString(parent->m_timeFormat).c_str());
			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				const Message& m = logger->getMessages()[i];
				msgItems[i]->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(parent->m_timeFormat).c_str());
			}
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
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(parent->m_timeFormat).c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, m.getText().c_str());

			line->setIcon((int)HeaderPos::contextName, Utilities::getIcon(m.getLevel()));
			line->setTextColor((int)HeaderPos::message, m.getColor().toQColor());
			QFont font = line->font((int)HeaderPos::message);
			font.setBold(true);
			line->setFont((int)HeaderPos::message, font);
			//line->setBackgroundColor((int)HeaderPos::message, m.getColor().toQColor());
			if (m.getContext())
			{
				QColor contextColor = (m.getContext()->getColor()*0.5).toQColor();
				line->setBackgroundColor((int)HeaderPos::contextName, contextColor);
				line->setBackgroundColor((int)HeaderPos::timestamp, contextColor);
				line->setBackgroundColor((int)HeaderPos::message, contextColor);
			}
			unsigned int levelIndex = (unsigned int)m.getLevel();
			if (levelIndex < static_cast<unsigned int>(Level::__count))
			{
				if (!parent->m_levelVisibility[levelIndex])
					line->setHidden(true);
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