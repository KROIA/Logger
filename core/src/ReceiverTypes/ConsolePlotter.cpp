#include "ReceiverTypes/ConsolePlotter.h"
#include <iostream>
#include <windows.h>

namespace Log
{
	namespace Receiver
	{
		ConsolePlotter::ConsolePlotter()
			: AbstractReceiver()
		{

		}
		ConsolePlotter::ConsolePlotter(const ConsolePlotter& other)
			: AbstractReceiver(other)
		{

		}

		ConsolePlotter::~ConsolePlotter()
		{

		}

		void ConsolePlotter::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			onPrintAllMessages(logger);
		}
		void ConsolePlotter::onUnsubscribed(Logger::AbstractLogger& logger)
		{

		}

		void ConsolePlotter::onNewMessage(const Message& m)
		{
			printToConsole(m);
		}
		void ConsolePlotter::onClear(Logger::AbstractLogger& logger)
		{

		}
		void ConsolePlotter::onDelete(Logger::AbstractLogger& loggerToDestroy)
		{

		}


		void ConsolePlotter::printToConsole(const Message& msg)
		{
			using std::cout;

			std::string type = getLevelStr(msg.getLevel());
			Logger::AbstractLogger* logger = msg.getContext();
			std::string contextName;
			if(logger)
				contextName = logger->getName();

			HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
			WORD wOldColorAttrs;
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

			/*
			* First save the current color information
			*/
			GetConsoleScreenBufferInfo(h, &csbiInfo);
			wOldColorAttrs = csbiInfo.wAttributes;
			
			int contextColor = Color::white.getConsoleValue();
			if(logger)
				contextColor = logger->getColor().getConsoleValue();
			int color = msg.getColor().getConsoleValue();
			
			cout << msg.getDateTime().toString() << "  ";
			SetConsoleTextAttribute(h, contextColor);
			cout << contextName << ": ";
			SetConsoleTextAttribute(h, color);
			cout << std::string(msg.getTabCount(), ' ') << type << msg.getText() << "\n";
			SetConsoleTextAttribute(h, wOldColorAttrs);
		}
	}
}