#include "ReceiverTypes/ui/QContextLoggerTree.h"

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

		void QContextLoggerTree::addContext(Logger::AbstractLogger& newContext)
		{
			if (m_msgItems.find(newContext.getID()) != m_msgItems.end())
				return;
			const Logger::ContextLogger* conextLogger = dynamic_cast<Logger::ContextLogger*>(&newContext);
			Logger::ContextLogger* parentLogger = nullptr;
			if(conextLogger)
			{
				parentLogger = conextLogger->getParent();
			}
			if (parentLogger)
			{
				auto& parentIt = m_msgItems.find(parentLogger->getID());
				if (parentIt == m_msgItems.end())
				{
					TreeData* treeData = new TreeData(this, newContext);
					m_msgItems[newContext.getID()] = treeData;
					return;
				}

				TreeData* parentTreeData = parentIt->second;
				m_msgItems[newContext.getID()] = parentTreeData->createChild(newContext);
				return;
			}			
			TreeData *treeData = new TreeData(this, newContext);
			m_msgItems[newContext.getID()] = treeData;
		}
		void QContextLoggerTree::removeContext(Logger::AbstractLogger::LoggerID id)
		{
			auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return;

			TreeData* treeData = it->second;
			m_msgItems.erase(it);
			delete treeData;
		}
		void QContextLoggerTree::onNewMessage(const Message& m)
		{
			auto &it = m_msgItems.find(m.getContext()->getID());
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->onNewMessage(m);
		}
		void QContextLoggerTree::clearMessages()
		{
			restartSearch:
			auto copy = m_msgItems;
			for(auto &it : copy)
			{
				if (!it.second->getLoggerIsAlive())
				{
					removeContext(it.first);
					goto restartSearch;
				}
				else
					it.second->clearMessages();
			}
		}
		void QContextLoggerTree::setContextVisibility(Logger::AbstractLogger::LoggerID id, bool isVisible)
		{
			auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->setContextVisibility(isVisible);
		}
		bool QContextLoggerTree::getContextVisibility(Logger::AbstractLogger::LoggerID id) const
		{
			auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return false;
			TreeData* treeData = it->second;
			return treeData->getContextVisibility();
		}
		void QContextLoggerTree::setLevelVisibility(Level level, bool isVisible)
		{
			if(level >= sizeof(m_levelVisibility) / sizeof(m_levelVisibility[0]))
				return;
			m_levelVisibility[level] = isVisible;
			for (auto& context : m_msgItems)
			{
				TreeData* treeData = context.second;
				treeData->setLevelVisibility(level, isVisible);
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
			for(auto & it : m_msgItems)
			{
				unsigned int count = 0;
				it.second->updateMessageCount(count);
			}
		}
		void QContextLoggerTree::updateMessageCount(unsigned int& countOut)
		{
			for(auto &it : m_msgItems)
			{
				unsigned int tmp = 0;
				it.second->updateMessageCount(tmp);
				countOut += tmp;
			}
		}



		QContextLoggerTree::TreeData::TreeData(QContextLoggerTree* root, const Logger::AbstractLogger& logger)
			: parent(nullptr)
		{
			this->root = root;
			
			childRoot = new QTreeWidgetItem(root->m_treeWidget);
			thisMessagesRoot = new QTreeWidgetItem(childRoot);

			loggerMetaInfo = logger.getMetaInfo();

			setupChildRoot();
			setupMessageRoot();
		}
		QContextLoggerTree::TreeData::TreeData(QContextLoggerTree* root, TreeData* parent, const Logger::AbstractLogger& logger)
			: parent(parent)
		{
			this->root = root;
			if (parent)
			{
				childRoot = new QTreeWidgetItem(parent->childRoot);
			}
			else
			{
				childRoot = new QTreeWidgetItem(root->m_treeWidget);
			}
			thisMessagesRoot = new QTreeWidgetItem(childRoot);

			loggerMetaInfo = logger.getMetaInfo();

			setupChildRoot();
			setupMessageRoot();
		}
		QContextLoggerTree::TreeData::~TreeData()
		{
			std::vector<TreeData*> _children = children;
			children.clear();
			for (auto& it : _children)
			{
				it->parent = nullptr;
				delete it;
			}
			if (parent)
			{
				auto& it = std::find(parent->children.begin(), parent->children.end(), this);
				if (it != parent->children.end())
					parent->children.erase(it);
			}
			if (root)
			{
				auto& it = root->m_msgItems.find(loggerMetaInfo->id);
				if (it != root->m_msgItems.end())
					root->m_msgItems.erase(it);
			}
			QTreeWidgetItem *childRoot_ = childRoot;
			childRoot = nullptr;
			thisMessagesRoot = nullptr;
			//detachLogger(*logger);
			delete childRoot_;
		}
		void QContextLoggerTree::TreeData::setupChildRoot()
		{
			QColor contextColor = loggerMetaInfo->color.toQColor();
			childRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, loggerMetaInfo->name.c_str());
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, loggerMetaInfo->creationTime.toString(root->m_timeFormat).c_str());
			childRoot->setBackgroundColor((int)HeaderPos::contextName, contextColor);
			childRoot->setBackgroundColor((int)HeaderPos::timestamp, contextColor);
			childRoot->setBackgroundColor((int)HeaderPos::message, contextColor);
		}
		void QContextLoggerTree::TreeData::setupMessageRoot()
		{
			QColor contextColor = loggerMetaInfo->color.toQColor();
			thisMessagesRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
			thisMessagesRoot->setBackgroundColor((int)HeaderPos::contextName, contextColor);
			thisMessagesRoot->setBackgroundColor((int)HeaderPos::timestamp, contextColor);
			thisMessagesRoot->setBackgroundColor((int)HeaderPos::message, contextColor);
		}
		void QContextLoggerTree::TreeData::updateDateTime()
		{
			//childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, logger->getCreationDateTime().toString(parent->m_timeFormat).c_str());
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, loggerMetaInfo->creationTime.toString(root->m_timeFormat).c_str());
			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				//const Message& m = logger->getMessages()[i];
				//msgItems[i]->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(parent->m_timeFormat).c_str());

				msgItems[i].item->setData((int)HeaderPos::timestamp, Qt::DisplayRole, msgItems[i].snapshot.dateTime.toString(root->m_timeFormat).c_str());
			}
		}

		void QContextLoggerTree::TreeData::onNewMessage(const Message& m)
		{
			QTreeWidgetItem* line = new QTreeWidgetItem(thisMessagesRoot);
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(root->m_timeFormat).c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, m.getText().c_str());

			line->setIcon((int)HeaderPos::contextName, Utilities::getIcon(m.getLevel()));
			line->setTextColor((int)HeaderPos::message, m.getColor().toQColor());
			QFont font = line->font((int)HeaderPos::message);
			font.setBold(true);
			line->setFont((int)HeaderPos::message, font);
			//line->setBackgroundColor((int)HeaderPos::message, m.getColor().toQColor());
			if (m.getContext())
			{
				QColor contextColor = (m.getContext()->getColor() * 0.5).toQColor();
				line->setBackgroundColor((int)HeaderPos::contextName, contextColor);
				line->setBackgroundColor((int)HeaderPos::timestamp, contextColor);
				line->setBackgroundColor((int)HeaderPos::message, contextColor);
			}
			unsigned int levelIndex = (unsigned int)m.getLevel();
			if (levelIndex < static_cast<unsigned int>(Level::__count))
			{
				if (!root->m_levelVisibility[levelIndex])
					line->setHidden(true);
			}

			MessageData data;
			data.item = line;
			data.snapshot = m.createSnapshot();

			msgItems.push_back(data);
			thisMessagesRoot->addChild(line);
		}
		QContextLoggerTree::TreeData* QContextLoggerTree::TreeData::createChild(Logger::AbstractLogger& newContext)
		{
			TreeData *child = new TreeData(root, this, newContext);
			children.push_back(child);
			return child;
		}
		void QContextLoggerTree::TreeData::getChildLoggerIDsRecursive(std::vector<Logger::AbstractLogger::LoggerID>& list) const
		{
			for (auto& it : children)
			{
				it->getLoggerIDsRecursive(list);
			}
		}
		void QContextLoggerTree::TreeData::getLoggerIDsRecursive(std::vector<Logger::AbstractLogger::LoggerID>& list) const
		{
			list.push_back(loggerMetaInfo->id);
			for (auto& it : children)
			{
				it->getLoggerIDsRecursive(list);
			}
		}

		void QContextLoggerTree::TreeData::setContextVisibility(bool isVisible)
		{
			childRoot->setHidden(!isVisible);
		}
		bool QContextLoggerTree::TreeData::getContextVisibility() const
		{
			return !childRoot->isHidden();
		}
		void QContextLoggerTree::TreeData::setLevelVisibility(Level level, bool isVisible)
		{
			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				if (msgItems[i].snapshot.level == level)
				{
					msgItems[i].item->setHidden(!isVisible);
				}
			}
		}
		void QContextLoggerTree::TreeData::updateMessageCount(unsigned int& countOut)
		{
			for(auto &it : children)
			{
				unsigned int tmp = 0;
				it->updateMessageCount(tmp);
				countOut += tmp;
			}
			countOut += msgItems.size();
			QString childsCountTxt = "[" + QString::number(countOut) + "] Messages childsCountTxt";
			childRoot->setData((int)HeaderPos::message, Qt::DisplayRole, childsCountTxt);

			QString messageCountTxt = "[" + QString::number(msgItems.size()) + "] Messages messageCountTxt";
			thisMessagesRoot->setData((int)HeaderPos::message, Qt::DisplayRole, messageCountTxt);
		}
		void QContextLoggerTree::TreeData::clearMessages()
		{
			for(auto &it : msgItems)
			{
				delete it.item;
			}
			msgItems.clear();
		}
		void QContextLoggerTree::TreeData::clearMessagesRecursive()
		{
			clearMessages();
			for(auto &it : children)
			{
				it->clearMessagesRecursive();
			}
		}
		bool QContextLoggerTree::TreeData::getLoggerIsAlive() const
		{
			return loggerMetaInfo->isAlive;
		}
		QContextLoggerTree::TreeData* QContextLoggerTree::TreeData::getParent() const
		{
			return parent;
		}
	}
}
#endif