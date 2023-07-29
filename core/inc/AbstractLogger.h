#pragma once

#include "Logger_base.h"
#include "LogMessage.h"

namespace Log
{
	class LOGGER_EXPORT AbstractLogger
	{
	public:
		AbstractLogger();
		AbstractLogger(const AbstractLogger& other);

		~AbstractLogger();

		void log(const Message& msg);

		void log(const std::string &msg);
		void log(const std::string& msg, Level level);
		void log(const std::string& msg, Level level, const Color& col);

		void log(const char* msg);
		void log(const char* msg, Level level);
		void log(const char* msg, Level level, const Color& col);

		void setTabCount(unsigned int tabCount);
		void tabIn();
		void tabOut();
		unsigned int getTabCount() const;

	protected:
		virtual void logInternal(const Message& msg) = 0;

		unsigned int m_tabCount;
	private:
	};
}