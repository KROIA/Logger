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
		// Preferred overload: caller supplies the LogObject::Info records explicitly
		// so we don't have to look them up in the LogManager singleton (which is
		// empty for file-loaded loggers, causing corrupt id=0 records on save).
		static bool saveToFile(const std::vector<LogObject::Info>& infos,
			const std::unordered_map<LoggerID, std::vector<Message>>& contexts,
			const std::string& file);
		
		
		static QJsonObject getLibraryInfo();
		static QJsonObject getLogLevelInfo();
		static QJsonObject getFileHeader();


	private:

	};
}