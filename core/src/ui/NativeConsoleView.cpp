#include "ui/NativeConsoleView.h"
#include "LogManager.h"
#include <iostream>
#include <windows.h>

namespace Log
{
	namespace UI
	{
		NativeConsoleView::NativeConsoleView()
			: AbstractReceiver()
			, m_dateTimeFormat(DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond)
		{

		}
		NativeConsoleView::NativeConsoleView(const NativeConsoleView& other)
			: AbstractReceiver()
			, m_dateTimeFormat(other.m_dateTimeFormat)
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
			m_contextCache[loggerInfo.id] = loggerInfo;
		}
		void NativeConsoleView::onLoggerInfoChanged(LogObject::Info info)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			m_contextCache[info.id] = info;
		}
		void NativeConsoleView::onLogMessage(Message message)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			const LogObject::Info& context = getContext(message.getLoggerID());
			if (context.visibilityPolicy == ReceiverVisibilityPolicy::Invisible)
				return;
			printToConsole(context, message);
		}
		void NativeConsoleView::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			const auto it = m_contextCache.find(childID);
			if (it != m_contextCache.end())
				it->second.parentId = newParentID;
		}

		const LogObject::Info& NativeConsoleView::getContext(LoggerID id)
		{
			const auto it = m_contextCache.find(id);
			if (it != m_contextCache.end())
				return it->second;
			// Not seen through the lifecycle slots (e.g. the logger existed
			// before this view did and the snapshot has not been delivered
			// yet). Pay the locked lookup once, then keep it.
			return m_contextCache.emplace(id, LogManager::getLogObjectInfo(id)).first->second;
		}


		void NativeConsoleView::printToConsole(const LogObject::Info& context, const Message& msg)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_2);
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