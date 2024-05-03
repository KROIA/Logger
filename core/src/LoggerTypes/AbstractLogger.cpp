#include "LoggerTypes/AbstractLogger.h"

namespace Log
{
	namespace Logger
	{
		DEFINE_SIGNAL_CONNECT_DISCONNECT(AbstractLogger, onNewMessage, const Message&);
		DEFINE_SIGNAL_CONNECT_DISCONNECT(AbstractLogger, onClear, AbstractLogger&);
		DEFINE_SIGNAL_CONNECT_DISCONNECT(AbstractLogger, onDelete, AbstractLogger&);


		AbstractLogger::AbstractLogger(const std::string& name)
			: LoggerInterface()
			, m_metaInfo(
				++getIDCounter(),
				0,
				name,
				DateTime(),
				Color::white,
				0,
				true,
				true)
			, onNewMessage("onNewMessage")
			, onClear("onClear")
			, onDelete("onDelete")
		{
			m_sharedMetaInfo = std::make_shared<MetaInfo>(m_metaInfo);
			getLoggerMap()[this] = m_sharedMetaInfo;
		}
		AbstractLogger::AbstractLogger(const AbstractLogger& other)
			: LoggerInterface()
			, m_metaInfo(other.m_metaInfo)
			, m_messages()
			, onNewMessage("onNewMessage")
			, onClear("onClear")
			, onDelete("onDelete")
		{
			m_metaInfo.id = ++getIDCounter();
			m_sharedMetaInfo = std::make_shared<MetaInfo>(m_metaInfo);
			m_messages.reserve(other.m_messages.size());
			for (size_t i = 0; i < other.m_messages.size(); ++i)
			{
				m_messages.push_back(other.m_messages[i]);
				m_messages.back().setContext(this);
			}
			getLoggerMap()[this] = m_sharedMetaInfo;
		}
		AbstractLogger::~AbstractLogger()
		{
			m_sharedMetaInfo->isAlive = false;
			onDelete.emitSignal(*this);
		}

		void AbstractLogger::setEnabled(bool enable)
		{
			m_metaInfo.enabled = enable;
			m_sharedMetaInfo->enabled = enable;
		}
		bool AbstractLogger::isEnabled() const
		{
			return m_metaInfo.enabled;
		}

		void AbstractLogger::setName(const std::string& name)
		{
			m_metaInfo.name = name;
			m_sharedMetaInfo->name = name;
		}
		const std::string& AbstractLogger::getName() const
		{
			return m_metaInfo.name;
		}
		void AbstractLogger::setColor(const Color& col)
		{
			m_metaInfo.color = col;
			m_sharedMetaInfo->color = col;
		}
		const Color& AbstractLogger::getColor() const
		{
			return m_metaInfo.color;
		}

		void AbstractLogger::log(Message msg)
		{
			if (!m_metaInfo.enabled) return;
			msg.setContext(this);
			logInternal(msg);
		}

		void AbstractLogger::log(const std::string& msg)
		{
			if (!m_metaInfo.enabled) return;
			Message m(msg);
			m.setTabCount(m_metaInfo.tabCount);
			m.setContext(this);
			logInternal(m);
		}
		void AbstractLogger::log(Level level, const std::string& msg)
		{
			if (!m_metaInfo.enabled) return;
			Message m(msg, level);
			m.setTabCount(m_metaInfo.tabCount);
			m.setContext(this);
			logInternal(m);
		}
		void AbstractLogger::log(Level level, const Color& col, const std::string& msg)
		{
			if (!m_metaInfo.enabled) return;
			Message m(msg, level, col);
			m.setTabCount(m_metaInfo.tabCount);
			m.setContext(this);
			logInternal(m);
		}

		void AbstractLogger::log(const char* msg)
		{
			if (!m_metaInfo.enabled) return;
			Message m(msg);
			m.setTabCount(m_metaInfo.tabCount);
			m.setContext(this);
			logInternal(m);
		}
		void AbstractLogger::log(Level level, const char* msg)
		{
			if (!m_metaInfo.enabled) return;
			Message m(msg, level);
			m.setTabCount(m_metaInfo.tabCount);
			m.setContext(this);
			logInternal(m);
		}
		void AbstractLogger::log(Level level, const Color& col, const char* msg)
		{
			if (!m_metaInfo.enabled) return;
			Message m(msg, level, col);
			m.setTabCount(m_metaInfo.tabCount);
			m.setContext(this);
			logInternal(m);
		}

		void AbstractLogger::setTabCount(unsigned int tabCount)
		{
			if (!m_metaInfo.enabled) return;
			m_metaInfo.tabCount = tabCount;
			m_sharedMetaInfo->tabCount = tabCount;
		}
		void AbstractLogger::tabIn()
		{
			if (!m_metaInfo.enabled) return;
			++m_metaInfo.tabCount;
			++m_sharedMetaInfo->tabCount;
		}
		void AbstractLogger::tabOut()
		{
			if (!m_metaInfo.enabled) return;
			if (m_metaInfo.tabCount > 0)
			{
				--m_metaInfo.tabCount;
				--m_sharedMetaInfo->tabCount;
			}
		}
		unsigned int AbstractLogger::getTabCount() const
		{
			return m_metaInfo.tabCount;
		}

#ifdef QT_WIDGETS_LIB
		void AbstractLogger::setIcon(const QIcon& icon)
		{
			m_icon = icon;
		}
		const QIcon& AbstractLogger::getIcon() const
		{
			return m_icon;		
		}
#endif

		void AbstractLogger::clear()
		{
			m_messages.clear();
			onClear.emitSignal(*this);
		}

		const DateTime& AbstractLogger::getCreationDateTime() const
		{
			return m_metaInfo.creationTime;
		}
		const std::vector<Message>& AbstractLogger::getMessages() const
		{
			return m_messages;
		}
		AbstractLogger::LoggerID AbstractLogger::getID() const
		{
			return m_metaInfo.id;
		}
		std::shared_ptr<const AbstractLogger::MetaInfo> AbstractLogger::getMetaInfo() const
		{
			return m_sharedMetaInfo;
		}
		std::shared_ptr<AbstractLogger::MetaInfo> AbstractLogger::getMetaInfo(const AbstractLogger* logger)
		{
			std::unordered_map<const AbstractLogger*, std::shared_ptr<AbstractLogger::MetaInfo>>& loggerMap = getLoggerMap();
			auto it = loggerMap.find(logger);
			if (it != loggerMap.end())
			{
				return it->second;
			}
			return nullptr;
		}

		void AbstractLogger::logInternal(const Message& msg)
		{
			m_messages.push_back(msg);
			onNewMessage.emitSignal(msg);
		}

		void AbstractLogger::emitNewMessage(const Message& msg)
		{
			onNewMessage.emitSignal(msg);
		}
		AbstractLogger::LoggerID& AbstractLogger::getIDCounter()
		{
			static LoggerID idCounter = 0;
			return idCounter;
		}
		std::unordered_map<const AbstractLogger*, std::shared_ptr<AbstractLogger::MetaInfo>>& AbstractLogger::getLoggerMap()
		{
			static std::unordered_map<const AbstractLogger*, std::shared_ptr<AbstractLogger::MetaInfo>> loggerMap;
			return loggerMap;
		}
	}
}