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
			, m_enable(true)
			, m_tabCount(0)
			, m_name(name)
			, m_creationsTime()
			, m_color(Color::white)
			, m_messages()
			, onNewMessage("onNewMessage")
			, onClear("onClear")
			, onDelete("onDelete")
		{

		}
		AbstractLogger::AbstractLogger(const AbstractLogger& other)
			: LoggerInterface()
			, m_enable(other.m_enable)
			, m_tabCount(other.m_tabCount)
			, m_name(other.m_name)
			, m_creationsTime(other.m_creationsTime)
			, m_color(other.m_color)
			, m_messages()
			, onNewMessage("onNewMessage")
			, onClear("onClear")
			, onDelete("onDelete")
		{
			m_messages.reserve(other.m_messages.size());
			for (size_t i = 0; i < other.m_messages.size(); ++i)
			{
				m_messages.push_back(other.m_messages[i]);
				m_messages.back().setContext(this);
			}
		}
		AbstractLogger::~AbstractLogger()
		{
			onDelete.emitSignal(*this);
		}

		void AbstractLogger::setEnabled(bool enable)
		{
			m_enable = enable;
		}
		bool AbstractLogger::isEnabled() const
		{
			return m_enable;
		}

		void AbstractLogger::setName(const std::string& name)
		{
			m_name = name;
		}
		const std::string& AbstractLogger::getName() const
		{
			return m_name;
		}
		void AbstractLogger::setColor(const Color& col)
		{
			m_color = col;
		}
		const Color& AbstractLogger::getColor() const
		{
			return m_color;
		}

		void AbstractLogger::log(Message msg)
		{
			if (!m_enable) return;
			msg.setContext(this);
			logInternal(msg);
		}

		void AbstractLogger::log(const std::string& msg)
		{
			if (!m_enable) return;
			Message m(msg);
			m.setTabCount(m_tabCount);
			m.setContext(this);
			logInternal(m);
		}
		void AbstractLogger::log(const std::string& msg, Level level)
		{
			if (!m_enable) return;
			Message m(msg, level);
			m.setTabCount(m_tabCount);
			m.setContext(this);
			logInternal(m);
		}
		void AbstractLogger::log(const std::string& msg, Level level, const Color& col)
		{
			if (!m_enable) return;
			Message m(msg, level, col);
			m.setTabCount(m_tabCount);
			m.setContext(this);
			logInternal(m);
		}

		void AbstractLogger::log(const char* msg)
		{
			if (!m_enable) return;
			Message m(msg);
			m.setTabCount(m_tabCount);
			m.setContext(this);
			logInternal(m);
		}
		void AbstractLogger::log(const char* msg, Level level)
		{
			if (!m_enable) return;
			Message m(msg, level);
			m.setTabCount(m_tabCount);
			m.setContext(this);
			logInternal(m);
		}
		void AbstractLogger::log(const char* msg, Level level, const Color& col)
		{
			if (!m_enable) return;
			Message m(msg, level, col);
			m.setTabCount(m_tabCount);
			m.setContext(this);
			logInternal(m);
		}

		void AbstractLogger::setTabCount(unsigned int tabCount)
		{
			if (!m_enable) return;
			m_tabCount = tabCount;
		}
		void AbstractLogger::tabIn()
		{
			if (!m_enable) return;
			++m_tabCount;
		}
		void AbstractLogger::tabOut()
		{
			if (!m_enable) return;
			if (m_tabCount > 0)
				--m_tabCount;
		}
		unsigned int AbstractLogger::getTabCount() const
		{
			return m_tabCount;
		}

		

		void AbstractLogger::clear()
		{
			m_messages.clear();
			onClear.emitSignal(*this);
		}

		const DateTime& AbstractLogger::getCreationDateTime() const
		{
			return m_creationsTime;
		}
		const std::vector<Message>& AbstractLogger::getMessages() const
		{
			return m_messages;
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