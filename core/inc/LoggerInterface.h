#pragma once

#include "Logger_base.h"
#include <string>
#include "LogMessage.h"
#include "LogLevel.h"

namespace Log
{
	class LoggerInterface
	{
	public:
		virtual ~LoggerInterface() {}

		virtual void log(Message msg) = 0;
		 
		virtual void log(const std::string& msg) = 0;
		virtual void log(Level level, const std::string& msg) = 0;
		virtual void log(Level level, const Color& col, const std::string& msg) = 0;

		virtual void logTrace(const std::string& msg) = 0;
		virtual void logDebug(const std::string& msg) = 0;
		virtual void logInfo(const std::string& msg) = 0;
		virtual void logWarning(const std::string& msg) = 0;
		virtual void logError(const std::string& msg) = 0;
		virtual void logCustom(const std::string& msg) = 0;
		 
		virtual void log(const char* msg) = 0;
		virtual void log(Level level, const char* msg) = 0;
		virtual void log(Level level, const Color& col, const char* msg) = 0;

		virtual void logTrace(const char* msg) = 0;
		virtual void logDebug(const char* msg) = 0;
		virtual void logInfo(const char* msg) = 0;
		virtual void logWarning(const char* msg) = 0;
		virtual void logError(const char* msg) = 0;
		virtual void logCustom(const char* msg) = 0;
		 
		virtual void setTabCount(unsigned int tabCount) = 0;
		virtual void tabIn() = 0;
		virtual void tabOut() = 0;
		virtual unsigned int getTabCount() const = 0;

	protected:

	private:
	};
}