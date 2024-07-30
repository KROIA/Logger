#include "ReceiverTypes/ConsoleContextPlotter.h"
#include "LogManager.h"
#include <iostream>
#include <windows.h>

namespace Log
{
	namespace UI
	{
		ConsoleContextPlotter::ConsoleContextPlotter()
			: AbstractReceiver()
			, m_dateTimeFormat(DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond)
		{

		}
		ConsoleContextPlotter::ConsoleContextPlotter(const ConsoleContextPlotter& other)
			: AbstractReceiver()
			, m_dateTimeFormat(other.m_dateTimeFormat)
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

		void ConsoleContextPlotter::onNewLogger(LogObject::Info loggerInfo)
		{
			LOGGER_UNUSED(loggerInfo);
		}
		void ConsoleContextPlotter::onLoggerInfoChanged(LogObject::Info info)
		{
			LOGGER_UNUSED(info);
		}
		void ConsoleContextPlotter::onLogMessage(Message message)
		{
			printToConsole(LogManager::getLogObjectInfo(message.getLoggerID()), message);
		}
		void ConsoleContextPlotter::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			LOGGER_UNUSED(childID);
			LOGGER_UNUSED(newParentID);
		}


		void ConsoleContextPlotter::printToConsole(const LogObject::Info& context, const Message& msg)
		{
			using std::cout;

			std::string type = Utilities::getLevelStr(msg.getLevel());
			type = type + ":" + std::string(10 - type.size(), ' ');

			HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
			WORD wOldColorAttrs;
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

			/*
			* First save the current color information
			*/
			GetConsoleScreenBufferInfo(h, &csbiInfo);
			wOldColorAttrs = csbiInfo.wAttributes;


			WORD contextColor = context.color.getConsoleValue();
			WORD color = (WORD)msg.getColor().getConsoleValue();

			cout << msg.getDateTime().toString(m_dateTimeFormat) << "  ";
			SetConsoleTextAttribute(h, contextColor);
			cout << context.name << ": ";
			SetConsoleTextAttribute(h, color);
			cout << type << msg.getText() << "\n";
			SetConsoleTextAttribute(h, wOldColorAttrs);
		}
	}
}