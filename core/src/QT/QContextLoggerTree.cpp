#include "QT/QContextLoggerTree.h"

#ifdef LOGGER_QT
#include <qdebug.h>

namespace Log
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

	void QContextLoggerTree::attachLogger(ContextLogger& logger)
	{
		logger.connect_onNewContext_slot(this, &QContextLoggerTree::onNewContext);
		logger.connect_onNewMessage_slot(this, &QContextLoggerTree::onNewMessage);
		logger.connect_onClear_slot(this, &QContextLoggerTree::onClear);
		logger.connect_onDestroyContext_slot(this, &QContextLoggerTree::onDestroyContext);
		logger.connect_onDelete_slot(this, &QContextLoggerTree::onContextDelete);

		onNewSubscribed(logger);
	}
	void QContextLoggerTree::detachLogger(ContextLogger& logger)
	{
		logger.disconnect_onNewContext_slot(this, &QContextLoggerTree::onNewContext);
		logger.disconnect_onNewMessage_slot(this, &QContextLoggerTree::onNewMessage);
		logger.disconnect_onClear_slot(this, &QContextLoggerTree::onClear);
		logger.disconnect_onDestroyContext_slot(this, &QContextLoggerTree::onDestroyContext);
		logger.disconnect_onDelete_slot(this, &QContextLoggerTree::onContextDelete);

		onUnsubscribed(logger);
	}
	void QContextLoggerTree::onNewSubscribed(const ContextLogger& logger)
	{
		auto& it = m_msgItems.find(&logger);
		if (it != m_msgItems.end())
			return;

		m_subscriber.push_back(&logger);

		TreeData treeData;
		treeData.logger = &logger;
		QTreeWidgetItem* newItem = new QTreeWidgetItem(m_treeWidget);
		QTreeWidgetItem* newMessageRoot = new QTreeWidgetItem(newItem);
		treeData.childRoot = newItem;
		treeData.thisMessagesRoot = newMessageRoot;

		newItem->setData((int)HeaderPos::contextName, Qt::DisplayRole, logger.getName().c_str());
		newItem->setData((int)HeaderPos::timestamp, Qt::DisplayRole, logger.getDateTime().toString().c_str());

		newMessageRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
		const std::vector<Message>& messages = logger.getMessages();
		for (size_t i = 0; i < messages.size(); ++i)
		{
			QTreeWidgetItem* line;
			line = new QTreeWidgetItem(newItem);
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, messages[i].getDateTime().toString().c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, messages[i].getText().c_str());

			treeData.msgItems.push_back(line);
			if (newItem)
				newItem->addChild(line);
		}
		m_msgItems[&logger] = treeData;

		const std::vector<ContextLogger*>& childs = logger.getChilds();
		for (size_t i = 0; i < childs.size(); ++i)
		{
			onNewContext(logger, *childs[i]);
		}
		

		unsigned int dummy;
		updateMessageCount(logger, dummy);
	}
	void QContextLoggerTree::onUnsubscribed(const ContextLogger& logger)
	{
		auto it = std::find(m_subscriber.begin(), m_subscriber.end(), &logger);
		if(it != m_subscriber.end())
			m_subscriber.erase(it);
	}
	void QContextLoggerTree::onNewContext(const ContextLogger& parent, const ContextLogger& newContext)
	{

		auto& parentIt = m_msgItems.find(&parent);
		if (parentIt == m_msgItems.end())
		{
			const ContextLogger* parentParent = parent.getParent();
			if (parentParent)
			{
				onNewContext(*parentParent, parent);
			}
		}
		

		parentIt = m_msgItems.find(&parent);
		if (parentIt == m_msgItems.end())
		{
			return;
		}

		if(m_msgItems.find(&newContext) != m_msgItems.end())
			return;

		QTreeWidgetItem* newItem = new QTreeWidgetItem(parentIt->second.childRoot);
		QTreeWidgetItem* newMessageRoot = new QTreeWidgetItem(newItem);
		TreeData treeData;
		treeData.logger = &newContext;
		treeData.childRoot = newItem;
		treeData.thisMessagesRoot = newMessageRoot;

		newItem->setData((int)HeaderPos::contextName, Qt::DisplayRole, newContext.getName().c_str());
		newItem->setData((int)HeaderPos::timestamp, Qt::DisplayRole, newContext.getDateTime().toString().c_str());

		newMessageRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");

		const std::vector<Message>& messages = newContext.getMessages();
		for (size_t i = 0; i < messages.size(); ++i)
		{
			QTreeWidgetItem* line;
			line = new QTreeWidgetItem(newItem);
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, messages[i].getDateTime().toString().c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, messages[i].getText().c_str());

			treeData.msgItems.push_back(line);
			if (newItem)
				newItem->addChild(line);
		}
		m_msgItems[&newContext] = treeData;
		unsigned int dummy;
		updateMessageCount(newContext, dummy);
	}
	void QContextLoggerTree::onNewMessage(const ContextLogger& logger, const Message& m)
	{
		auto &it = m_msgItems.find(&logger);
		if (it == m_msgItems.end())
		{
			const ContextLogger* parent = logger.getParent();
			if (parent)
			{
				onNewContext(*parent, logger);
			}
		}

		it = m_msgItems.find(&logger);
		if (it == m_msgItems.end())
			return;
		TreeData& treeData = it->second;

		QTreeWidgetItem* line = new QTreeWidgetItem(treeData.thisMessagesRoot);
		line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString().c_str());
		line->setData((int)HeaderPos::message, Qt::DisplayRole, m.getText().c_str());

		line->setIcon((int)HeaderPos::contextName, getIcon(m.getLevel()));
		line->setBackgroundColor((int)HeaderPos::message, m.getColor().toQColor());

		treeData.msgItems.push_back(line);
		treeData.thisMessagesRoot->addChild(line);

		unsigned int dummy;
		updateMessageCount(logger, dummy);
	}
	void QContextLoggerTree::onClear(const ContextLogger& logger)
	{
		auto &it = m_msgItems.find(&logger);
		if (it == m_msgItems.end())
			return;
		TreeData &treeData = it->second;

		for (size_t i = 0; i < treeData.msgItems.size(); ++i)
			delete treeData.msgItems[i];
		treeData.msgItems.clear();
		unsigned int dummy;
		updateMessageCount(logger, dummy);
	}
	void QContextLoggerTree::onDestroyContext(const ContextLogger& loggerToDestroy)
	{
		auto& it = m_msgItems.find(&loggerToDestroy);
		if (it == m_msgItems.end())
			return;
		TreeData treeData = it->second;
		
		delete treeData.childRoot;
		m_msgItems.erase(it);
	}
	void QContextLoggerTree::onContextDelete(ContextLogger& loggerToDestroy)
	{
		detachLogger(loggerToDestroy);
	}
	void QContextLoggerTree::onUpdateTimer()
	{
		unsigned int dummy = 0;
		for (const ContextLogger* logger : m_subscriber)
		{
			updateMessageCountRecursive(*logger, dummy);
		}
	}
	void QContextLoggerTree::updateMessageCount(const ContextLogger& logger, unsigned int& countOut)
	{
		auto& it = m_msgItems.find(&logger);
		if (it == m_msgItems.end())
			return;
		TreeData& treeData = it->second;
		countOut = logger.getMessages().size();
		QString text = "[" + QString::number(countOut) + "] Messages";
		treeData.thisMessagesRoot->setData((int)HeaderPos::message, Qt::DisplayRole, text);
	}
	void QContextLoggerTree::updateMessageCountRecursive(const ContextLogger& logger, unsigned int& countOut)
	{
		auto& it = m_msgItems.find(&logger);
		if (it == m_msgItems.end())
			return;
		TreeData& treeData = it->second;
		updateMessageCount(logger, countOut);

		const std::vector<ContextLogger*>& childs = logger.getChilds();
		for (size_t i = 0; i < childs.size(); ++i)
		{
			unsigned int childCount;
			updateMessageCountRecursive(*childs[i], childCount);
			countOut += childCount;
		}
		QString text = "[" + QString::number(countOut) + "] Messages";
		treeData.childRoot->setData((int)HeaderPos::message, Qt::DisplayRole, text);
	}

}
#endif