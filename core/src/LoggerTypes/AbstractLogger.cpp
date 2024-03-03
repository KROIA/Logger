#include "LoggerTypes/AbstractLogger.h"

namespace Log
{
	namespace Logger
	{
		DEFINE_SIGNAL_CONNECT_DISCONNECT(AbstractLogger, onNewMessage, const Message&);
		DEFINE_SIGNAL_CONNECT_DISCONNECT(AbstractLogger, onClear, AbstractLogger&);
		DEFINE_SIGNAL_CONNECT_DISCONNECT(AbstractLogger, onDelete, AbstractLogger&);

		int AbstractLogger::s_idCounter = 0;

		AbstractLogger::AbstractLogger(const std::string& name)
			: LoggerInterface()
			, m_metaInfo(
				++s_idCounter,
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
			m_sharedMetaInfo = std::make_shared<LoggerMetaInfo>(m_metaInfo);
		}
		AbstractLogger::AbstractLogger(const AbstractLogger& other)
			: LoggerInterface()
			, m_metaInfo(other.m_metaInfo)
			, m_messages()
			, onNewMessage("onNewMessage")
			, onClear("onClear")
			, onDelete("onDelete")
		{
			m_metaInfo.id = ++s_idCounter;
			m_sharedMetaInfo = std::make_shared<LoggerMetaInfo>(m_metaInfo);
			m_messages.reserve(other.m_messages.size());
			for (size_t i = 0; i < other.m_messages.size(); ++i)
			{
				m_messages.push_back(other.m_messages[i]);
				m_messages.back().setContext(this);
			}
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
		std::shared_ptr<const AbstractLogger::LoggerMetaInfo> AbstractLogger::getMetaInfo() const
		{
			return m_sharedMetaInfo;
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
	}
}