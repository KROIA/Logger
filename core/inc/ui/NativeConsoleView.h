#pragma once
#include "Logger_base.h"
#include "AbstractReceiver.h"
#include "Utilities/DateTime.h"
#include "LogMessage.h"
#include <QObject>

namespace Log
{
	namespace UI
	{
		class LOGGER_API NativeConsoleView : public AbstractReceiver
		{
		public:
			NativeConsoleView();
			NativeConsoleView(const NativeConsoleView& other);

			~NativeConsoleView();

			static void createStaticInstance();
			static void destroyStaticInstance();
			static NativeConsoleView*& getStaticInstance();

			void setDateTimeFormat(DateTime::Format format);
			DateTime::Format getDateTimeFormat() const;

			void hide();
			void show();
			bool isVisible() const;
			void clear();

			// Emits each log line with a single write instead of four coloured segments,
			// so foreign writers on the same stdout can no longer land text inside a log
			// line. Colour is kept via ANSI escapes, and dropped automatically when stdout
			// is redirected or the console has no virtual terminal support.
			// Defaults to true if the environment variable LOGGER_CONSOLE_ATOMIC_LINES is set.
			void setAtomicLines(bool enable);
			bool getAtomicLines() const;

			// ANSI colour inside the atomic line. Defaults to whether stdout is a console
			// with virtual terminal support, detected at construction; force it off to keep
			// escapes out of a captured log, or on for a terminal this cannot detect.
			void setAtomicLineColors(bool enable);
			bool getAtomicLineColors() const;


		private:
			void onNewLogger(LogObject::Info loggerInfo) override;
			void onLoggerInfoChanged(LogObject::Info info) override;
			void onLogMessage(Message message) override;
			void onChangeParent(LoggerID childID, LoggerID newParentID) override;
			

			void printToConsole(const LogObject::Info &context, const Message& msg);

			DateTime::Format m_dateTimeFormat;
			bool m_atomicLines;
			bool m_atomicLineColors;
		};
	}
}