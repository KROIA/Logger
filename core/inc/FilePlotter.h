#pragma once
#include "Logger_base.h"
#include "LogMessage.h"
#include "AbstractReceiver.h"
#include <QFile>
#include <QJsonObject>
#include <QTextStream>

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
		void insertJson(const QJsonValue& obj);
		void createDirectoryIfNotExists(const QString& filePath);

		std::string m_filePath;
		QFile *m_file;
		// One stream for the lifetime of the file instead of constructing and
		// destructing one (plus re-applying the UTF-8 codec) on every single
		// message. Still flushed per write, so a crash loses nothing that the
		// previous implementation would have kept.
		QTextStream m_stream;
		DateTime::Format m_dateTimeFormat;
	};
}