#pragma once
#include "Logger_base.h"
#include "LogMessage.h"
#include "LogObject.h"

#include <QJsonArray>
#include <QJsonObject>
#include <unordered_map>

namespace Log
{
	class LOGGER_API Export
	{
	public:
		static bool saveToFile(const std::unordered_map<LoggerID, std::vector<Message>>& contexts, const std::string& file);
		
		
		static QJsonObject toJson(const LogObject::Info& info);
		static QJsonObject toJson(const Message& message);
		static QJsonObject getLibraryInfo();
		static QJsonObject getLogLevelInfo();
		static QJsonObject getFileHeader();


	private:

	};
}