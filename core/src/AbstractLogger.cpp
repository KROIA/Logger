#include "AbstractLogger.h"

namespace Log
{
	AbstractLogger::AbstractLogger(const std::string& name)
		: LoggerInterface()
		, m_tabCount(0)
		, m_name(name)
		, m_enable(true)
	{

	}
	AbstractLogger::AbstractLogger(const AbstractLogger& other)
		: LoggerInterface()
		, m_tabCount(other.m_tabCount)
		, m_enable(other.m_enable)
	{

	}
	AbstractLogger::~AbstractLogger()
	{

	}

	void AbstractLogger::setName(const std::string& name)
	{
		m_name = name;
	}
	const std::string& AbstractLogger::getName() const
	{
		return m_name;
	}

	void AbstractLogger::log(const Message& msg)
	{
		if (!m_enable) return;
		logInternal(msg);
	}

	void AbstractLogger::log(const std::string& msg)
	{
		if (!m_enable) return;
		logInternal(Message(msg));
	}
	void AbstractLogger::log(const std::string& msg, Level level)
	{
		if (!m_enable) return;
		logInternal(Message(msg, level));
	}
	void AbstractLogger::log(const std::string& msg, Level level, const Color& col)
	{
		if (!m_enable) return;
		logInternal(Message(msg, level, col));
	}

	void AbstractLogger::log(const char* msg)
	{
		if (!m_enable) return;
		logInternal(Message(msg));
	}
	void AbstractLogger::log(const char* msg, Level level)
	{
		if (!m_enable) return;
		logInternal(Message(msg, level));
	}
	void AbstractLogger::log(const char* msg, Level level, const Color& col)
	{
		if (!m_enable) return;
		logInternal(Message(msg, level, col));
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

	const std::string AbstractLogger::getLevelStr(Level l)
	{
		switch (l)
		{
			case Level::trace: return " Trace: ";
			case Level::debug: return " Debug: ";
			case Level::info: return " Info: ";
			case Level::warning: return " Warning: ";
			case Level::error: return " Error: ";
		}
		return "Unknown debug level: "+std::to_string(l);
	}
	void AbstractLogger::setEnabled(bool enable)
	{
		m_enable = enable;
	}
	bool AbstractLogger::isEnabled() const
	{
		return m_enable;
	}
}