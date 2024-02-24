#include "ReceiverTypes/ConsoleContextPlotter.h"
#include <iostream>
#include <windows.h>

namespace Log
{
	namespace Receiver
	{
		ConsoleContextPlotter::ConsoleContextPlotter()
			: ContextReceiver()
		{

		}
		ConsoleContextPlotter::ConsoleContextPlotter(const ConsoleContextPlotter& other)
			: ContextReceiver(other)
		{

		}

		ConsoleContextPlotter::~ConsoleContextPlotter()
		{

		}

		void ConsoleContextPlotter::onContextCreate(Logger::ContextLogger& logger)
		{

		}
		void ConsoleContextPlotter::onContextDestroy(Logger::ContextLogger& logger)
		{

		}

		void ConsoleContextPlotter::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			Logger::ContextLogger *context = dynamic_cast<Logger::ContextLogger*>(&logger);
			if (context)
				onPrintAllMessagesRecursive(*context);
		}
		void ConsoleContextPlotter::onUnsubscribed(Logger::AbstractLogger& logger)
		{

		}

		void ConsoleContextPlotter::onNewMessage(const Message& m)
		{
			printToConsole(m);
		}
		void ConsoleContextPlotter::onClear(Logger::AbstractLogger& logger)
		{

		}
		void ConsoleContextPlotter::onDelete(Logger::AbstractLogger& loggerToDestroy)
		{

		}


		void ConsoleContextPlotter::printToConsole(const Message& msg)
		{
			using std::cout;

			std::string type = getLevelStr(msg.getLevel());
			Logger::AbstractLogger* logger = msg.getContext();
			std::string contextName;
			if (logger)
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
			if (logger)
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