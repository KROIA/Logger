#include "Logger.h"

namespace Log
{
	Logger::Logger()
		: m_tabCount(0)
	{

	}
	Logger::Logger(const Logger& other)
		: m_tabCount(other.m_tabCount)
	{

	}

	Logger::~Logger()
	{

	}

	void Logger::log(const Message& msg)
	{
		logInternal(msg);
	}

	void Logger::log(const std::string& msg)
	{
		logInternal(Message(msg));
	}
	void Logger::log(const std::string& msg, Level level)
	{
		logInternal(Message(msg, level));
	}
	void Logger::log(const std::string& msg, Level level, const Color& col)
	{
		logInternal(Message(msg, level, col));
	}

	void Logger::log(const char* msg)
	{
		logInternal(Message(msg));
	}
	void Logger::log(const char* msg, Level level)
	{
		logInternal(Message(msg, level));
	}
	void Logger::log(const char* msg, Level level, const Color& col)
	{
		logInternal(Message(msg, level, col));
	}

	void Logger::setTabCount(unsigned int tabCount)
	{
		m_tabCount = tabCount;
	}
	void Logger::tabIn()
	{
		++m_tabCount;
	}
	void Logger::tabOut()
	{
		if (m_tabCount > 0)
			--m_tabCount;
	}
	unsigned int Logger::getTabCount() const
	{
		return m_tabCount;
	}
}