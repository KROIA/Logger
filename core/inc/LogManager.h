#pragma once

#include "Logger_base.h"
#include <QObject>
#include <unordered_map>
#include "LogMessage.h"
#include "LogObject.h"
#include <Mutex>

namespace Log
{
	class LogManager: public QObject
	{
		friend class LogObject;
		Q_OBJECT
		LogManager();

		static LoggerID addNewLogger(LogObject::Info loggerInfo);
		static void onLogMessageInternal(Message message);
		static void onChangeParentInternal(LoggerID childID, LoggerID newParentID);

		static void setLogObjectInfo(LogObject::Info info);
	public:

		static LogManager& instance();

		static LogObject::Info getLogObjectInfo(LoggerID loggerID);
		static std::vector<LogObject::Info> getLogObjectsInfo();

	signals:
		void onNewLogger(LogObject::Info loggerInfo);
		void onLoggerInfoChanged(LogObject::Info info);
		void onLogMessage(Message message);
		void onChangeParent(LoggerID childID, LoggerID newParentID);


	private:
		std::mutex m_mutex;
		std::unordered_map<LoggerID, LogObject::Info> m_logObjects;
		LoggerID m_nextID;
	};
}