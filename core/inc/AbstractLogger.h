#pragma once

#include "Logger_base.h"
#include "LogMessage.h"
#include "LoggerInterface.h"

namespace Log
{
	class LOGGER_EXPORT AbstractLogger: public LoggerInterface
	{
	public:
		AbstractLogger(const std::string &name = "");
		AbstractLogger(const AbstractLogger& other);

		~AbstractLogger();

		void setName(const std::string& name);
		const std::string& getName() const;

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

		const std::string getLevelStr(Level l);

		void setEnabled(bool enable);
		bool isEnabled() const;
	protected:
		virtual void logInternal(const Message& msg) = 0;

		unsigned int m_tabCount;
		std::string m_name;

		bool m_enable;
	private:

		 
	};
}