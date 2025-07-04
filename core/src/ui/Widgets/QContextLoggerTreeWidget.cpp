#include "ui/Widgets/QContextLoggerTreeWidget.h"
#include "LogManager.h"
#include <algorithm>

#ifdef QT_WIDGETS_LIB

namespace Log
{
	namespace UIWidgets
	{
		QContextLoggerTreeWidget::QContextLoggerTreeWidget(QTreeWidget* parent)
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
			connect(&m_updateTimer, &QTimer::timeout, this, &QContextLoggerTreeWidget::onUpdateTimer);
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

		QContextLoggerTreeWidget::~QContextLoggerTreeWidget()
		{

		}

		const QString& QContextLoggerTreeWidget::getHeaderName(HeaderPos pos) const
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
		unsigned int QContextLoggerTreeWidget::getHeaderWidth(HeaderPos pos) const
		{
			switch (pos)
			{
			case HeaderPos::contextName: { return 200; }
			case HeaderPos::timestamp: { return 150; }
			case HeaderPos::message: { return 500; }
			}
			return 0;
		}
		void QContextLoggerTreeWidget::setDateTimeFormat(DateTime::Format format)
		{
			if(m_timeFormat == format)
				return;
			m_timeFormat = format;
			for (auto& context : m_msgItems)
			{
				context.second->updateDateTime();
			}
		}
		DateTime::Format QContextLoggerTreeWidget::getDateTimeFormat() const
		{
			return m_timeFormat;
		}

		void QContextLoggerTreeWidget::addContext(const LogObject::Info &newContext)
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
		void QContextLoggerTreeWidget::onNewMessage(const Message& m)
		{
			const auto &it = m_msgItems.find(m.getLoggerID());
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->onNewMessage(m);
		}
		void QContextLoggerTreeWidget::clearMessages()
		{
			for (auto& it : m_msgItems)
			{
				it.second->clearMessages();
			}
		}


		void QContextLoggerTreeWidget::setDateTimeFilter(const DateTimeFilter& filter)
		{
			m_dateTimeFilter = filter;
			updateDateTimeFilter();
		}
		const DateTimeFilter& QContextLoggerTreeWidget::getDateTimeFilter() const
		{
			return m_dateTimeFilter;
		}
		void QContextLoggerTreeWidget::setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType)
		{
			m_dateTimeFilter.min = min;
			m_dateTimeFilter.max = max;
			m_dateTimeFilter.rangeType = rangeType;
			m_dateTimeFilter.active = true;
			updateDateTimeFilter();
		}
		void QContextLoggerTreeWidget::clearDateTimeFilter()
		{
			m_dateTimeFilter.active = false;
			updateDateTimeFilter();
		}
		const DateTime& QContextLoggerTreeWidget::getDateTimeFilterMin() const
		{
			return m_dateTimeFilter.min;
		}
		const DateTime& QContextLoggerTreeWidget::getDateTimeFilterMax() const
		{
			return m_dateTimeFilter.max;
		}
		DateTime::Range QContextLoggerTreeWidget::getDateTimeFilterRangeType() const
		{
			return m_dateTimeFilter.rangeType;
		}
		bool QContextLoggerTreeWidget::isDateTimeFilterActive() const
		{
			return m_dateTimeFilter.active;
		}
		void QContextLoggerTreeWidget::setParent(LoggerID childID, LoggerID parentID)
		{
			TreeData* child = nullptr;
			TreeData* parent = nullptr;
			
			const auto &childIt = m_msgItems.find(childID);
			if (childIt != m_msgItems.end())
				child = childIt->second;

			const auto &parentIt = m_msgItems.find(parentID);
			if (parentIt != m_msgItems.end())
				parent = parentIt->second;
			
			if (child && parent)
			{
				child->setParent(parent);
			}
		}
		void QContextLoggerTreeWidget::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const
		{
			for (auto& it : m_msgItems)
			{
				it.second->saveVisibleMessages(list);
			}
		}


		void QContextLoggerTreeWidget::setContextVisibility(LoggerID id, bool isVisible)
		{
			const auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->setContextVisibility(isVisible);
		}
		bool QContextLoggerTreeWidget::getContextVisibility(LoggerID id) const
		{
			const auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return false;
			TreeData* treeData = it->second;
			return treeData->getContextVisibility();
		}
		void QContextLoggerTreeWidget::setLevelVisibility(Level level, bool isVisible)
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
		bool QContextLoggerTreeWidget::getLevelVisibility(Level level) const
		{
			if (level >= sizeof(m_levelVisibility) / sizeof(m_levelVisibility[0]))
				return false;
			return m_levelVisibility[level];
		}
		void QContextLoggerTreeWidget::onUpdateTimer()
		{
			for(auto & it : m_msgItems)
			{
				unsigned int count = 0;
				it.second->updateMessageCount(count);
			}
		}
		void QContextLoggerTreeWidget::updateMessageCount(unsigned int& countOut)
		{
			for(auto &it : m_msgItems)
			{
				unsigned int tmp = 0;
				it.second->updateMessageCount(tmp);
				countOut += tmp;
			}
		}
		void QContextLoggerTreeWidget::updateDateTimeFilter()
		{
			for (auto& it : m_msgItems)
			{
				TreeData* treeData = it.second;
				treeData->updateDateTimeFilter(m_dateTimeFilter);
			}
		}



		QContextLoggerTreeWidget::TreeData::TreeData(QContextLoggerTreeWidget* root, LoggerID loggerID)
			: parent(nullptr)
		{
			this->root = root;
			
			childRoot = new QTreeWidgetItem(root->m_treeWidget);
			thisMessagesRoot = new QTreeWidgetItem(childRoot);

			this->loggerID = loggerID;

			setupChildRoot();
			setupMessageRoot();
		}
		QContextLoggerTreeWidget::TreeData::TreeData(QContextLoggerTreeWidget* root, TreeData* parent, LoggerID loggerID)
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
		QContextLoggerTreeWidget::TreeData::~TreeData()
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
		void QContextLoggerTreeWidget::TreeData::setupChildRoot()
		{
			LogObject::Info info = LogManager::getLogObjectInfo(loggerID);
			QColor contextColor = info.color.toQColor();
			childRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, info.name.c_str());
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, info.creationTime.toString(root->m_timeFormat).c_str());
			childRoot->setBackground((int)HeaderPos::contextName, contextColor);
			childRoot->setBackground((int)HeaderPos::timestamp, contextColor);
			childRoot->setBackground((int)HeaderPos::message, contextColor);
		}
		void QContextLoggerTreeWidget::TreeData::setupMessageRoot()
		{
			LogObject::Info info = LogManager::getLogObjectInfo(loggerID);
			QColor contextColor = info.color.toQColor();
			thisMessagesRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
			thisMessagesRoot->setBackground((int)HeaderPos::contextName, contextColor);
			thisMessagesRoot->setBackground((int)HeaderPos::timestamp, contextColor);
			thisMessagesRoot->setBackground((int)HeaderPos::message, contextColor);
		}
		void QContextLoggerTreeWidget::TreeData::updateDateTime()
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

		void QContextLoggerTreeWidget::TreeData::onNewMessage(const Message& m)
		{
			QTreeWidgetItem* line = new QTreeWidgetItem(thisMessagesRoot);
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(root->m_timeFormat).c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, m.getText().c_str());

			line->setIcon((int)HeaderPos::contextName, Utilities::getIcon(m.getLevel()));
			line->setForeground((int)HeaderPos::message, m.getColor().toQColor());
			QFont font = line->font((int)HeaderPos::message);
			font.setBold(true);
			line->setFont((int)HeaderPos::message, font);

			line->setToolTip((int)HeaderPos::message, m.getText().c_str());
			line->setToolTip((int)HeaderPos::timestamp, m.getDateTime().toString(root->m_timeFormat).c_str());
			line->setToolTip((int)HeaderPos::contextName, m.getLevelString().c_str());

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
		QContextLoggerTreeWidget::TreeData* QContextLoggerTreeWidget::TreeData::createChild(LoggerID loggerID)
		{
			TreeData *child = new TreeData(root, this, loggerID);
			children.push_back(child);
			return child;
		}


		// Function to move a QTreeWidgetItem to a new parent
		static void changeParent(QTreeWidgetItem* item, QTreeWidgetItem* newParent) {
			if (item == nullptr || newParent == nullptr) return;

			// Get the current parent
			QTreeWidgetItem* currentParent = item->parent();

			if (currentParent) {
				// If the item has a parent, remove it from that parent
				currentParent->takeChild(currentParent->indexOfChild(item));
			}
			else {
				// If the item is a top-level item, remove it from the QTreeWidget directly
				QTreeWidget* treeWidget = item->treeWidget();
				if (treeWidget) {
					treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(item));
				}
			}

			// Add the item to the new parent
			newParent->addChild(item);
		}
		void QContextLoggerTreeWidget::TreeData::setParent(TreeData* newParent)
		{
			auto old_parent = childRoot->parent();
			if (old_parent != NULL) 
			{ 
				auto ix = old_parent->indexOfChild(childRoot);
				auto item_without_parent = old_parent->takeChild(ix); 
				newParent->childRoot->addChild(item_without_parent); 
			}
			else
			{
				//newParent->childRoot->addChild(childRoot);
				
			}



			if (parent)
			{
				const auto& it = std::find(parent->children.begin(), parent->children.end(), this);
				if (it != parent->children.end())
					parent->children.erase(it);
			}
			changeParent(childRoot, newParent->childRoot);
			parent = newParent;
			if (parent)
			{
				parent->children.push_back(this);
				//parent->childRoot->addChild(childRoot);
			}
		}
		/*void QContextLoggerTreeWidget::TreeData::changeParent(LoggerID childID, TreeData* newParent)
		{
			if(!newParent)
				return;
			for(size_t i=0; i<children.size(); ++i)
			{
				if(children[i]->loggerID == childID)
				{
					TreeData* child = children[i];
					children.erase(children.begin() + i);
					child->parent = newParent;
					newParent->children.push_back(child);
					return;
				}
			}
		}*/
		void QContextLoggerTreeWidget::TreeData::getChildLoggerIDsRecursive(std::vector<LoggerID>& list) const
		{
			for (auto& it : children)
			{
				it->getLoggerIDsRecursive(list);
			}
		}
		void QContextLoggerTreeWidget::TreeData::getLoggerIDsRecursive(std::vector<LoggerID>& list) const
		{
			list.push_back(loggerID);
			for (auto& it : children)
			{
				it->getLoggerIDsRecursive(list);
			}
		}

		void QContextLoggerTreeWidget::TreeData::setContextVisibility(bool isVisible)
		{
			childRoot->setHidden(!isVisible);
		}
		bool QContextLoggerTreeWidget::TreeData::getContextVisibility() const
		{
			return !childRoot->isHidden();
		}
		void QContextLoggerTreeWidget::TreeData::setLevelVisibility(Level level, bool isVisible)
		{
			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				if (msgItems[i].msg.getLevel() == level)
				{
					msgItems[i].setVisibilityFilter(MessageData::VisibilityBitMask::levelVisibility, isVisible);
				}
			}
		}
		void QContextLoggerTreeWidget::TreeData::updateMessageCount(unsigned int& countOut)
		{
			for(auto &it : children)
			{
				unsigned int tmp = 0;
				it->updateMessageCount(tmp);
				countOut += tmp;
			}
			countOut += msgItems.size();
			QString childsCountTxt = "[" + QString::number(countOut) + "] Messages";
			childRoot->setData((int)HeaderPos::message, Qt::DisplayRole, childsCountTxt);

			QString messageCountTxt = "[" + QString::number(msgItems.size()) + "] Messages";
			thisMessagesRoot->setData((int)HeaderPos::message, Qt::DisplayRole, messageCountTxt);
		}
		void QContextLoggerTreeWidget::TreeData::clearMessages()
		{
			for(auto &it : msgItems)
			{
				delete it.item;
			}
			msgItems.clear();
		}
		void QContextLoggerTreeWidget::TreeData::clearMessagesRecursive()
		{
			clearMessages();
			for(auto &it : children)
			{
				it->clearMessagesRecursive();
			}
		}
		//bool QContextLoggerTreeWidget::TreeData::getLoggerIsAlive() const
		//{
		//	return MetaInfo->isAlive;
		//}
		QContextLoggerTreeWidget::TreeData* QContextLoggerTreeWidget::TreeData::getParent() const
		{
			return parent;
		}
		void QContextLoggerTreeWidget::TreeData::updateDateTimeFilter(const DateTimeFilter& filter)
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
		void QContextLoggerTreeWidget::TreeData::saveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const
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