#include "ConsoleLogger.h"
#include <iostream>
#include <windows.h>

namespace Log
{
	ConsoleLogger::ConsoleLogger()
		: AbstractLogger()
	{

	}
	ConsoleLogger::ConsoleLogger(const ConsoleLogger& other)
		: AbstractLogger(other)
	{

	}

	ConsoleLogger::~ConsoleLogger()
	{

	}

	void ConsoleLogger::logInternal(const Message& msg)
	{
		using std::cout;

		std::string type;
		switch (msg.getLevel())
		{
			case Level::trace: type = " Trace: "; break;
			case Level::debug: type = " Debug: "; break;
			case Level::info: type  = " Info: "; break;
			case Level::warning: type  = " Warning: "; break;
			case Level::error: type  = " Error: "; break;
		}

		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		WORD wOldColorAttrs;
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

		/*
		* First save the current color information
		*/
		GetConsoleScreenBufferInfo(h, &csbiInfo);
		wOldColorAttrs = csbiInfo.wAttributes;

		int color = msg.getColor().getConsoleValue();
		cout << msg.getDateTime().toString() << "  ";
		SetConsoleTextAttribute(h, color);
		cout << std::string(m_tabCount, ' ') << type << msg.getText() << "\n";
		SetConsoleTextAttribute(h, wOldColorAttrs);
	}
}