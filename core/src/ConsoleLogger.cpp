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

		std::string type = getLevelStr(msg.getLevel());

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
		cout << std::string(m_tabCount, ' ') << m_name << ": " << type << msg.getText() << "\n";
		SetConsoleTextAttribute(h, wOldColorAttrs);
	}
}