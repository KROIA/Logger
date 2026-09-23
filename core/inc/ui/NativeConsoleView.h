#pragma once
#include "Logger_base.h"
#include "AbstractReceiver.h"
#include "Utilities/DateTime.h"
#include "LogMessage.h"
#include <QObject>
#include <unordered_map>

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
			

		private:
			void onNewLogger(LogObject::Info loggerInfo) override;
			void onLoggerInfoChanged(LogObject::Info info) override;
			void onLogMessage(Message message) override;
			void onChangeParent(LoggerID childID, LoggerID newParentID) override;
			

			void printToConsole(const LogObject::Info &context, const Message& msg);

			// Context lookup for the message being printed. Kept locally so a
			// message does not cost a locked LogManager query plus a full
			// LogObject::Info copy (string included) every time. Fed by the
			// logger lifecycle slots; a miss falls back to LogManager once.
			const LogObject::Info& getContext(LoggerID id);

			DateTime::Format m_dateTimeFormat;
			std::unordered_map<LoggerID, LogObject::Info> m_contextCache;
		};
	}
}