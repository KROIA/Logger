#pragma once
#include "Logger_base.h"
#include "LogMessage.h"
#include "AbstractReceiver.h"
#include <QFile>
#include <QJsonObject>

namespace Log
{
	class LOGGER_API FilePlotter : public AbstractReceiver
	{
	public:
		FilePlotter(const std::string& filePath, DateTime::Format format = DateTime::Format::hourMinuteSecondMillisecond | DateTime::Format::yearMonthDay);
		~FilePlotter();

	protected:
		void onNewLogger(LogObject::Info loggerInfo) override;
		void onLoggerInfoChanged(LogObject::Info info) override;
		void onLogMessage(Message message) override;
		void onChangeParent(LoggerID childID, LoggerID newParentID) override;
	private:
		void insertObject(const QJsonObject& obj);

		std::string m_filePath;
		QFile *m_file;
		DateTime::Format m_dateTimeFormat;
	};
}