#pragma once

#include "Logger_base.h"
#include <string>

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
		 
		virtual void log(const char* msg) = 0;
		virtual void log(Level level, const char* msg) = 0;
		virtual void log(Level level, const Color& col, const char* msg) = 0;
		 
		virtual void setTabCount(unsigned int tabCount) = 0;
		virtual void tabIn() = 0;
		virtual void tabOut() = 0;
		virtual unsigned int getTabCount() const = 0;

	protected:

	private:
	};
}