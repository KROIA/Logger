#pragma once

#include "Logger_base.h"
#include "Utilities/MessageFilter.h"
#include <unordered_set>
#include <vector>

namespace Log
{
	class Message;
	class LOGGER_API LoggerIDFilter : public MessageFilter
	{
	public:
		enum Mode
		{
			Include,
			Exclude
		};
		LoggerIDFilter() = default;
		LoggerIDFilter(const LoggerIDFilter& other);
		virtual ~LoggerIDFilter() = default;

		bool operator==(const LoggerIDFilter& other) const;
		bool operator!=(const LoggerIDFilter& other) const;

		LoggerIDFilter& operator=(const LoggerIDFilter& other);


		void setMode(Mode mode) { m_mode = mode; }
		Mode getMode() const { return m_mode; }

		void addLoggerID(LoggerID id) { m_loggerIDs.insert(id); }
		void removeLoggerID(LoggerID id) { m_loggerIDs.erase(id); }
		bool containsLoggerID(LoggerID id) const { return m_loggerIDs.find(id) != m_loggerIDs.end(); }
		void clearLoggerIDs() { m_loggerIDs.clear(); }
		void setLoggerIDs(const std::unordered_set<LoggerID>& ids) { m_loggerIDs = ids; }
		void setLoggerIDs(const std::vector<LoggerID>& ids) { m_loggerIDs.insert(ids.begin(), ids.end()); }
		const std::unordered_set<LoggerID>& getLoggerIDs() const { return m_loggerIDs; }
		
		/**
		* @return true if the message should be accepted by the receiver,
		*         false if it should be filtered out
		*/
		bool filter(const Message& message) const override;

	private:
		Mode m_mode = Mode::Include;
		std::unordered_set<LoggerID> m_loggerIDs;
	};
}