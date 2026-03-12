#include "Filter/LoggerIDFilter.h"
#include "LogMessage.h"
#include "LogManager.h"

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
		LoggerID msgLoggerID = message.getLoggerID();
		bool contains = containsLoggerID(msgLoggerID);
		if(!contains && m_includeChildren)
		{
			for (const auto& id : m_loggerIDs)
			{
				if (LogManager::isChildOf(msgLoggerID, id))
				{
					contains = true;
					break;
				}
			}
		}
		if (m_mode == Mode::Include)
			return contains;
		else // Exclude
			return !contains;
	}

}