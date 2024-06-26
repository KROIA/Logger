#include "ui/QContextLoggerTree.h"
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
				const auto& parentIt = m_msgItems.find(parentLogger->getID());
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
			const auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return;

			TreeData* treeData = it->second;
			m_msgItems.erase(it);
			delete treeData;
		}
		void QContextLoggerTree::onNewMessage(const Message& m)
		{
			const auto &it = m_msgItems.find(m.getContext()->getID());
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
		void QContextLoggerTree::getSaveVisibleMessages(std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list) const
		{
			for (auto& it : m_msgItems)
			{
				it.second->saveVisibleMessages(list);
			}
		}


		void QContextLoggerTree::setContextVisibility(Logger::AbstractLogger::LoggerID id, bool isVisible)
		{
			const auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->setContextVisibility(isVisible);
		}
		bool QContextLoggerTree::getContextVisibility(Logger::AbstractLogger::LoggerID id) const
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



		QContextLoggerTree::TreeData::TreeData(QContextLoggerTree* root, const Logger::AbstractLogger& logger)
			: parent(nullptr)
		{
			this->root = root;
			
			childRoot = new QTreeWidgetItem(root->m_treeWidget);
			thisMessagesRoot = new QTreeWidgetItem(childRoot);

			MetaInfo = logger.getMetaInfo();

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

			MetaInfo = logger.getMetaInfo();

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
				const auto& it = root->m_msgItems.find(MetaInfo->id);
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
			QColor contextColor = MetaInfo->color.toQColor();
			childRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, MetaInfo->name.c_str());
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, MetaInfo->creationTime.toString(root->m_timeFormat).c_str());
			childRoot->setBackground((int)HeaderPos::contextName, contextColor);
			childRoot->setBackground((int)HeaderPos::timestamp, contextColor);
			childRoot->setBackground((int)HeaderPos::message, contextColor);
		}
		void QContextLoggerTree::TreeData::setupMessageRoot()
		{
			QColor contextColor = MetaInfo->color.toQColor();
			thisMessagesRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
			thisMessagesRoot->setBackground((int)HeaderPos::contextName, contextColor);
			thisMessagesRoot->setBackground((int)HeaderPos::timestamp, contextColor);
			thisMessagesRoot->setBackground((int)HeaderPos::message, contextColor);
		}
		void QContextLoggerTree::TreeData::updateDateTime()
		{
			//childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, logger->getCreationDateTime().toString(parent->m_timeFormat).c_str());
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, MetaInfo->creationTime.toString(root->m_timeFormat).c_str());
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
			line->setForeground((int)HeaderPos::message, m.getColor().toQColor());
			QFont font = line->font((int)HeaderPos::message);
			font.setBold(true);
			line->setFont((int)HeaderPos::message, font);
			//line->setBackgroundColor((int)HeaderPos::message, m.getColor().toQColor());
			if (m.getContext())
			{
				QColor contextColor = (m.getContext()->getColor() * 0.5).toQColor();
				line->setBackground((int)HeaderPos::contextName, contextColor);
				line->setBackground((int)HeaderPos::timestamp, contextColor);
				line->setBackground((int)HeaderPos::message, contextColor);
			}
			

			MessageData data;
			data.item = line;
			data.snapshot = m.createSnapshot();

			unsigned int levelIndex = (unsigned int)m.getLevel();
			if (levelIndex < static_cast<unsigned int>(Level::__count))
			{
				if (!root->m_levelVisibility[levelIndex])
					data.setVisibilityFilter(MessageData::VisibilityBitMask::levelVisibility, false);
			}
			if (!root->m_dateTimeFilter.matches(data.snapshot.dateTime))
				data.setVisibilityFilter(MessageData::VisibilityBitMask::dateTimeVisibility, false);

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
			list.push_back(MetaInfo->id);
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
		bool QContextLoggerTree::TreeData::getLoggerIsAlive() const
		{
			return MetaInfo->isAlive;
		}
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
					elementIsVisible = filter.matches(msgItem.snapshot.dateTime);
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
		void QContextLoggerTree::TreeData::saveVisibleMessages(std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list) const
		{
			Logger::AbstractLogger::LoggerSnapshotData snapshot(*MetaInfo);
			snapshot.messages.reserve(msgItems.size());

			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				if (msgItems[i].isVisible())
				{
					snapshot.messages.push_back(msgItems[i].snapshot);
				}
			}
			list.push_back(snapshot);
		}
	}
}
#endif