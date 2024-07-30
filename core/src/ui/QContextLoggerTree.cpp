#include "ui/QContextLoggerTree.h"
#include "LogManager.h"
#include <algorithm>

#ifdef QT_WIDGETS_LIB

namespace Log
{
	namespace Receiver
	{
		QContextLoggerTree::QContextLoggerTree(QTreeWidget* parent)
			: QWidget(parent)
			, m_treeWidget(parent)
		{
			for(int i = 0; i < sizeof(m_levelVisibility) / sizeof(m_levelVisibility[0]); ++i)
			{
				m_levelVisibility[i] = true;
			}

			m_dateTimeFilter.active = false;
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

		void QContextLoggerTree::addContext(const LogObject::Info &newContext)
		{
			if (m_msgItems.find(newContext.id) != m_msgItems.end())
				return;

			LoggerID parentID = newContext.parentId;

			if (parentID > 0)
			{
				const auto& parentIt = m_msgItems.find(parentID);
				if (parentIt == m_msgItems.end())
				{
					TreeData* treeData = new TreeData(this, newContext.id);
					m_msgItems[newContext.id] = treeData;
					return;
				}

				TreeData* parentTreeData = parentIt->second;
				m_msgItems[newContext.id] = parentTreeData->createChild(newContext.id);
				return;
			}			
			TreeData *treeData = new TreeData(this, newContext.id);
			m_msgItems[newContext.id] = treeData;
		}
		void QContextLoggerTree::onNewMessage(const Message& m)
		{
			const auto &it = m_msgItems.find(m.getLoggerID());
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->onNewMessage(m);
		}
		void QContextLoggerTree::clearMessages()
		{
			for (auto& it : m_msgItems)
			{
				it.second->clearMessages();
			}
		}


		void QContextLoggerTree::setDateTimeFilter(const DateTimeFilter& filter)
		{
			m_dateTimeFilter = filter;
			updateDateTimeFilter();
		}
		const DateTimeFilter& QContextLoggerTree::getDateTimeFilter() const
		{
			return m_dateTimeFilter;
		}
		void QContextLoggerTree::setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType)
		{
			m_dateTimeFilter.min = min;
			m_dateTimeFilter.max = max;
			m_dateTimeFilter.rangeType = rangeType;
			m_dateTimeFilter.active = true;
			updateDateTimeFilter();
		}
		void QContextLoggerTree::clearDateTimeFilter()
		{
			m_dateTimeFilter.active = false;
			updateDateTimeFilter();
		}
		const DateTime& QContextLoggerTree::getDateTimeFilterMin() const
		{
			return m_dateTimeFilter.min;
		}
		const DateTime& QContextLoggerTree::getDateTimeFilterMax() const
		{
			return m_dateTimeFilter.max;
		}
		DateTime::Range QContextLoggerTree::getDateTimeFilterRangeType() const
		{
			return m_dateTimeFilter.rangeType;
		}
		bool QContextLoggerTree::isDateTimeFilterActive() const
		{
			return m_dateTimeFilter.active;
		}
		void QContextLoggerTree::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const
		{
			for (auto& it : m_msgItems)
			{
				it.second->saveVisibleMessages(list);
			}
		}


		void QContextLoggerTree::setContextVisibility(LoggerID id, bool isVisible)
		{
			const auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->setContextVisibility(isVisible);
		}
		bool QContextLoggerTree::getContextVisibility(LoggerID id) const
		{
			const auto& it = m_msgItems.find(id);
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
		void QContextLoggerTree::updateDateTimeFilter()
		{
			for (auto& it : m_msgItems)
			{
				TreeData* treeData = it.second;
				treeData->updateDateTimeFilter(m_dateTimeFilter);
			}
		}



		QContextLoggerTree::TreeData::TreeData(QContextLoggerTree* root, LoggerID loggerID)
			: parent(nullptr)
		{
			this->root = root;
			
			childRoot = new QTreeWidgetItem(root->m_treeWidget);
			thisMessagesRoot = new QTreeWidgetItem(childRoot);

			this->loggerID = loggerID;

			setupChildRoot();
			setupMessageRoot();
		}
		QContextLoggerTree::TreeData::TreeData(QContextLoggerTree* root, TreeData* parent, LoggerID loggerID)
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

			this->loggerID = loggerID;

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
				const auto& it = std::find(parent->children.begin(), parent->children.end(), this);
				if (it != parent->children.end())
					parent->children.erase(it);
			}
			if (root)
			{
				const auto& it = root->m_msgItems.find(loggerID);
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
			LogObject::Info info = LogManager::getLogObjectInfo(loggerID);
			QColor contextColor = info.color.toQColor();
			childRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, info.name.c_str());
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, info.creationTime.toString(root->m_timeFormat).c_str());
			childRoot->setBackground((int)HeaderPos::contextName, contextColor);
			childRoot->setBackground((int)HeaderPos::timestamp, contextColor);
			childRoot->setBackground((int)HeaderPos::message, contextColor);
		}
		void QContextLoggerTree::TreeData::setupMessageRoot()
		{
			LogObject::Info info = LogManager::getLogObjectInfo(loggerID);
			QColor contextColor = info.color.toQColor();
			thisMessagesRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
			thisMessagesRoot->setBackground((int)HeaderPos::contextName, contextColor);
			thisMessagesRoot->setBackground((int)HeaderPos::timestamp, contextColor);
			thisMessagesRoot->setBackground((int)HeaderPos::message, contextColor);
		}
		void QContextLoggerTree::TreeData::updateDateTime()
		{
			//childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, logger->getCreationDateTime().toString(parent->m_timeFormat).c_str());
			LogObject::Info info = LogManager::getLogObjectInfo(loggerID);
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, info.creationTime.toString(root->m_timeFormat).c_str());
			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				//const Message& m = logger->getMessages()[i];
				//msgItems[i]->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(parent->m_timeFormat).c_str());

				msgItems[i].item->setData((int)HeaderPos::timestamp, Qt::DisplayRole, msgItems[i].msg.getDateTime().toString(root->m_timeFormat).c_str());
			}
		}

		void QContextLoggerTree::TreeData::onNewMessage(const Message& m)
		{
			QTreeWidgetItem* line = new QTreeWidgetItem(thisMessagesRoot);
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(root->m_timeFormat).c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, m.getText().c_str());

			line->setIcon((int)HeaderPos::contextName, Utilities::getIcon(m.getLevel()));
			line->setForeground((int)HeaderPos::message, m.getColor().toQColor());
			QFont font = line->font((int)HeaderPos::message);
			font.setBold(true);
			line->setFont((int)HeaderPos::message, font);
			//line->setBackgroundColor((int)HeaderPos::message, m.getColor().toQColor());
			if (m.getLoggerID() > 0)
			{
				QColor contextColor = (LogManager::getLogObjectInfo(m.getLoggerID()).color * 0.5).toQColor();
				line->setBackground((int)HeaderPos::contextName, contextColor);
				line->setBackground((int)HeaderPos::timestamp, contextColor);
				line->setBackground((int)HeaderPos::message, contextColor);
			}
			

			MessageData data;
			data.item = line;
			data.msg = m;

			unsigned int levelIndex = (unsigned int)m.getLevel();
			if (levelIndex < static_cast<unsigned int>(Level::__count))
			{
				if (!root->m_levelVisibility[levelIndex])
					data.setVisibilityFilter(MessageData::VisibilityBitMask::levelVisibility, false);
			}
			if (!root->m_dateTimeFilter.matches(data.msg.getDateTime()))
				data.setVisibilityFilter(MessageData::VisibilityBitMask::dateTimeVisibility, false);

			msgItems.push_back(data);
			thisMessagesRoot->addChild(line);
		}
		QContextLoggerTree::TreeData* QContextLoggerTree::TreeData::createChild(LoggerID loggerID)
		{
			TreeData *child = new TreeData(root, this, loggerID);
			children.push_back(child);
			return child;
		}
		void QContextLoggerTree::TreeData::getChildLoggerIDsRecursive(std::vector<LoggerID>& list) const
		{
			for (auto& it : children)
			{
				it->getLoggerIDsRecursive(list);
			}
		}
		void QContextLoggerTree::TreeData::getLoggerIDsRecursive(std::vector<LoggerID>& list) const
		{
			list.push_back(loggerID);
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
				if (msgItems[i].msg.getLevel() == level)
				{
					msgItems[i].setVisibilityFilter(MessageData::VisibilityBitMask::levelVisibility, isVisible);
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
		//bool QContextLoggerTree::TreeData::getLoggerIsAlive() const
		//{
		//	return MetaInfo->isAlive;
		//}
		QContextLoggerTree::TreeData* QContextLoggerTree::TreeData::getParent() const
		{
			return parent;
		}
		void QContextLoggerTree::TreeData::updateDateTimeFilter(const DateTimeFilter& filter)
		{
			
			if (filter.active)
			{
				for (size_t i = 0; i < msgItems.size(); ++i)
				{
					bool elementIsVisible = true;
					MessageData &msgItem = msgItems[i];
					elementIsVisible = filter.matches(msgItem.msg.getDateTime());
					//elementIsVisible = 0;
					msgItem.setVisibilityFilter(MessageData::VisibilityBitMask::dateTimeVisibility, elementIsVisible);
				}
			}
			else
			{
				for (size_t i = 0; i < msgItems.size(); ++i)
				{
					msgItems[i].setVisibilityFilter(MessageData::VisibilityBitMask::dateTimeVisibility, true);
				}
			}
		}
		void QContextLoggerTree::TreeData::saveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const
		{
			std::vector<Message> messages;
			messages.reserve(msgItems.size());

			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				if (msgItems[i].isVisible())
				{
					messages.push_back(msgItems[i].msg);
				}
			}
			list[loggerID] = messages;
		}
	}
}
#endif