#pragma once

#include "Logger_base.h"
#include "LogMessage.h"
#include "LoggerInterface.h"

namespace Log
{
	class LOGGER_EXPORT AbstractLogger: public LoggerInterface
	{
	public:
		AbstractLogger();
		AbstractLogger(const AbstractLogger& other);

		
		void log(const Message& msg) override;

		void log(const std::string &msg) override;
		void log(const std::string& msg, Level level) override;
		void log(const std::string& msg, Level level, const Color& col) override;

		void log(const char* msg) override;
		void log(const char* msg, Level level) override;
		void log(const char* msg, Level level, const Color& col) override;

		void setTabCount(unsigned int tabCount) override;
		void tabIn() override;
		void tabOut() override;
		unsigned int getTabCount() const override;

	protected:
		virtual void logInternal(const Message& msg) = 0;

		unsigned int m_tabCount;
	private:
	};
}