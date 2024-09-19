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
		class LOGGER_EXPORT NativeConsoleView : public AbstractReceiver
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

		private:
			void onNewLogger(LogObject::Info loggerInfo) override;
			void onLoggerInfoChanged(LogObject::Info info) override;
			void onLogMessage(Message message) override;
			void onChangeParent(LoggerID childID, LoggerID newParentID) override;
			

			void printToConsole(const LogObject::Info &context, const Message& msg);

			DateTime::Format m_dateTimeFormat;
		};
	}
}