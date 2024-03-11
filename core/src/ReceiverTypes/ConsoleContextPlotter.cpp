#include "ReceiverTypes/ConsoleContextPlotter.h"
#include <iostream>
#include <windows.h>

namespace Log
{
	namespace Receiver
	{
		ConsoleContextPlotter::ConsoleContextPlotter()
			: ContextReceiver()
			, m_dateTimeFormat(DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond)
		{

		}
		ConsoleContextPlotter::ConsoleContextPlotter(const ConsoleContextPlotter& other)
			: ContextReceiver(other)
			, m_dateTimeFormat(DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond)
		{

		}

		ConsoleContextPlotter::~ConsoleContextPlotter()
		{

		}

		void ConsoleContextPlotter::setDateTimeFormat(DateTime::Format format)
		{
			m_dateTimeFormat = format;
		}
		DateTime::Format ConsoleContextPlotter::getDateTimeFormat() const
		{
			return m_dateTimeFormat;
		}

		void ConsoleContextPlotter::onContextCreate(Logger::ContextLogger& logger)
		{
			LOGGER_UNUSED(logger);
		}
		void ConsoleContextPlotter::onContextDestroy(Logger::AbstractLogger& logger)
		{
			LOGGER_UNUSED(logger);
		}

		void ConsoleContextPlotter::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			Logger::ContextLogger *context = dynamic_cast<Logger::ContextLogger*>(&logger);
			if (context)
				onPrintAllMessagesRecursive(*context);
			else
				onPrintAllMessages(logger);
		}
		void ConsoleContextPlotter::onUnsubscribed(Logger::AbstractLogger& logger)
		{
			LOGGER_UNUSED(logger);
		}

		void ConsoleContextPlotter::onNewMessage(const Message& m)
		{
			printToConsole(m);
		}
		void ConsoleContextPlotter::onClear(Logger::AbstractLogger& logger)
		{
			LOGGER_UNUSED(logger);
		}
		void ConsoleContextPlotter::onDelete(Logger::AbstractLogger& loggerToDestroy)
		{
			LOGGER_UNUSED(loggerToDestroy);
		}


		void ConsoleContextPlotter::printToConsole(const Message& msg)
		{
			using std::cout;

			std::string type = Utilities::getLevelStr(msg.getLevel());
			type = type + ":" + std::string(10 - type.size(), ' ');
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


			WORD contextColor = (WORD)Color::white.getConsoleValue();
			if (logger)
				contextColor = (WORD)logger->getColor().getConsoleValue();
			WORD color = (WORD)msg.getColor().getConsoleValue();

			cout << msg.getDateTime().toString(m_dateTimeFormat) << "  ";
			SetConsoleTextAttribute(h, contextColor);
			cout << contextName << ": ";
			SetConsoleTextAttribute(h, color);
			cout << std::string(msg.getTabCount(), ' ') << type << msg.getText() << "\n";
			SetConsoleTextAttribute(h, wOldColorAttrs);
		}
	}
}