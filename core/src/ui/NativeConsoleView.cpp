#include "ui/NativeConsoleView.h"
#include "LogManager.h"
#include <iostream>
#include <string>
#include <QtGlobal>
#include <windows.h>

namespace
{
	// Atomic-line mode cannot use SetConsoleTextAttribute: that colours by changing state
	// *between* writes, which is exactly what forces the line to be split. ANSI escapes
	// travel inside the string instead, so colour survives a single write.
	// Off when stdout is not a console (redirected to a file/pipe) or when the console
	// refuses virtual terminal processing - escapes would then land in the log as garbage.
	bool ansiColorAvailable()
	{
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode = 0;
		if (h == INVALID_HANDLE_VALUE || !GetConsoleMode(h, &mode))
			return false; // redirected, not a console
		if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
			return true;
		return SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
	}

	std::string ansiFg(const Log::Color& c)
	{
		return "\033[38;2;" + std::to_string((int)c.getRed()) + ";" +
			std::to_string((int)c.getGreen()) + ";" +
			std::to_string((int)c.getBlue()) + "m";
	}
}

namespace Log
{
	namespace UI
	{
		NativeConsoleView::NativeConsoleView()
			: AbstractReceiver()
			, m_dateTimeFormat(DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond)
			, m_atomicLines(qEnvironmentVariableIsSet("LOGGER_CONSOLE_ATOMIC_LINES"))
			, m_atomicLineColors(ansiColorAvailable())
		{

		}
		NativeConsoleView::NativeConsoleView(const NativeConsoleView& other)
			: AbstractReceiver()
			, m_dateTimeFormat(other.m_dateTimeFormat)
			, m_atomicLines(other.m_atomicLines)
			, m_atomicLineColors(other.m_atomicLineColors)
		{

		}

		NativeConsoleView::~NativeConsoleView()
		{

		}

		void NativeConsoleView::createStaticInstance()
		{
			NativeConsoleView*& instancePtr = getStaticInstance();
			if (instancePtr)
				return;
			instancePtr = new NativeConsoleView();
		}
		void NativeConsoleView::destroyStaticInstance()
		{
			NativeConsoleView*& instancePtr = getStaticInstance();
			if (instancePtr)
			{
				delete instancePtr;
				instancePtr = nullptr;
			}
		}
		NativeConsoleView*& NativeConsoleView::getStaticInstance()
		{
			static NativeConsoleView* instancePtr = nullptr;
			return instancePtr;
		}

		void NativeConsoleView::setDateTimeFormat(DateTime::Format format)
		{
			m_dateTimeFormat = format;
		}
		DateTime::Format NativeConsoleView::getDateTimeFormat() const
		{
			return m_dateTimeFormat;
		}

		void NativeConsoleView::setAtomicLines(bool enable)
		{
			m_atomicLines = enable;
		}
		bool NativeConsoleView::getAtomicLines() const
		{
			return m_atomicLines;
		}
		void NativeConsoleView::setAtomicLineColors(bool enable)
		{
			m_atomicLineColors = enable;
		}
		bool NativeConsoleView::getAtomicLineColors() const
		{
			return m_atomicLineColors;
		}

		void NativeConsoleView::hide()
		{
			::ShowWindow(::GetConsoleWindow(), SW_HIDE);
		}
		void NativeConsoleView::show()
		{
			::ShowWindow(::GetConsoleWindow(), SW_SHOW);
		}
		bool NativeConsoleView::isVisible() const
		{
			return ::IsWindowVisible(::GetConsoleWindow());
		}
		void NativeConsoleView::clear()
		{
			HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			GetConsoleScreenBufferInfo(h, &csbiInfo);
			DWORD dwConSize = csbiInfo.dwSize.X * csbiInfo.dwSize.Y;
			COORD coordScreen = { 0, 0 };
			DWORD cCharsWritten;
			FillConsoleOutputCharacter(h, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
			GetConsoleScreenBufferInfo(h, &csbiInfo);
			FillConsoleOutputAttribute(h, csbiInfo.wAttributes, dwConSize, coordScreen, &cCharsWritten);
			SetConsoleCursorPosition(h, coordScreen);
		}

		void NativeConsoleView::onNewLogger(LogObject::Info loggerInfo)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			LOGGER_UNUSED(loggerInfo);
		}
		void NativeConsoleView::onLoggerInfoChanged(LogObject::Info info)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			LOGGER_UNUSED(info);
		}
		void NativeConsoleView::onLogMessage(Message message)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			const LogObject::Info context = LogManager::getLogObjectInfo(message.getLoggerID());
			if (context.visibilityPolicy == ReceiverVisibilityPolicy::Invisible)
				return;
			printToConsole(context, message);
		}
		void NativeConsoleView::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			LOGGER_UNUSED(childID);
			LOGGER_UNUSED(newParentID);
		}


		void NativeConsoleView::printToConsole(const LogObject::Info& context, const Message& msg)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_2);
			using std::cout;

			std::string type = Utilities::getLevelStr(msg.getLevel());
			type = type + ":" + std::string(10 - type.size(), ' ');

			if (m_atomicLines)
			{
				// One write, so a foreign writer on the same stdout can only interleave
				// between lines, never inside one. Colour is carried by ANSI escapes
				// inside the string (dropped when stdout is not a VT console).
				std::string line = msg.getDateTime().toString(m_dateTimeFormat) + "  ";
				if (m_atomicLineColors)
					line += ansiFg(context.color) + context.name + ": " +
							ansiFg(msg.getColor()) + type + msg.getText() + "\033[0m\n";
				else
					line += context.name + ": " + type + msg.getText() + "\n";
				cout << line;
				return;
			}

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