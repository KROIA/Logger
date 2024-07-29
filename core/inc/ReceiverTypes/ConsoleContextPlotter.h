#pragma once
#include "Logger_base.h"
#include "ReceiverTypes/AbstractReceiver.h"
#include "Utilities/DateTime.h"
#include "LogMessage.h"
#include <QObject>

namespace Log
{
	namespace Receiver
	{
		class LOGGER_EXPORT ConsoleContextPlotter : public AbstractReceiver
		{
		public:
			ConsoleContextPlotter();
			ConsoleContextPlotter(const ConsoleContextPlotter& other);

			~ConsoleContextPlotter();

			void setDateTimeFormat(DateTime::Format format);
			DateTime::Format getDateTimeFormat() const;

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