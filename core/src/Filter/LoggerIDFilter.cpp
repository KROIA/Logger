#include "Filter/LoggerIDFilter.h"
#include "LogMessage.h"

namespace Log
{
	LoggerIDFilter::LoggerIDFilter(const LoggerIDFilter& other)
		: m_mode(other.m_mode)
		, m_loggerIDs(other.m_loggerIDs)
	{
	}

	bool LoggerIDFilter::operator==(const LoggerIDFilter& other) const
	{
		return m_mode == other.m_mode && 
			m_loggerIDs == other.m_loggerIDs;
	}
	bool LoggerIDFilter::operator!=(const LoggerIDFilter& other) const
	{
		return !(*this == other);
	}

	LoggerIDFilter& LoggerIDFilter::operator=(const LoggerIDFilter& other)
	{
		if (this == &other)
			return *this;
		m_mode = other.m_mode;
		m_loggerIDs = other.m_loggerIDs;
		return *this;
	}
	bool LoggerIDFilter::filter(const Message& message) const
	{
		bool contains = containsLoggerID(message.getLoggerID());
		if (m_mode == Mode::Include)
			return contains;
		else // Exclude
			return !contains;
	}

}