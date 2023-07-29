#include "AbstractLogger.h"

namespace Log
{
	AbstractLogger::AbstractLogger()
		: m_tabCount(0)
	{

	}
	AbstractLogger::AbstractLogger(const AbstractLogger& other)
		: m_tabCount(other.m_tabCount)
	{

	}

	AbstractLogger::~AbstractLogger()
	{

	}

	void AbstractLogger::log(const Message& msg)
	{
		logInternal(msg);
	}

	void AbstractLogger::log(const std::string& msg)
	{
		logInternal(Message(msg));
	}
	void AbstractLogger::log(const std::string& msg, Level level)
	{
		logInternal(Message(msg, level));
	}
	void AbstractLogger::log(const std::string& msg, Level level, const Color& col)
	{
		logInternal(Message(msg, level, col));
	}

	void AbstractLogger::log(const char* msg)
	{
		logInternal(Message(msg));
	}
	void AbstractLogger::log(const char* msg, Level level)
	{
		logInternal(Message(msg, level));
	}
	void AbstractLogger::log(const char* msg, Level level, const Color& col)
	{
		logInternal(Message(msg, level, col));
	}

	void AbstractLogger::setTabCount(unsigned int tabCount)
	{
		m_tabCount = tabCount;
	}
	void AbstractLogger::tabIn()
	{
		++m_tabCount;
	}
	void AbstractLogger::tabOut()
	{
		if (m_tabCount > 0)
			--m_tabCount;
	}
	unsigned int AbstractLogger::getTabCount() const
	{
		return m_tabCount;
	}
}