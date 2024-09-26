#include "Utilities/Export.h"
#include <fstream>

#include "Logger_info.h"
#include "LogManager.h"

#include <QJsonDocument>


namespace Log
{
	bool Export::saveToFile(
		const std::unordered_map<LoggerID, std::vector<Message>>& contexts, 
		const std::string& file)
	{
		QJsonArray objs;

		objs.append(getFileHeader());
		for(const auto& context : contexts)
		{
			const LogObject::Info &metaInfo = LogManager::getLogObjectInfo(context.first);
			objs.append(toJson(metaInfo));
		}
		std::vector<Message> messages;
		for (const auto& context : contexts)
		{
			for (const auto& message : context.second)
			{
				messages.push_back(message);
			}
		}
		// Sort messages by date and time
		std::sort(messages.begin(), messages.end(), [](const Message& a, const Message& b) 
			{ return a.getDateTime() < b.getDateTime(); });

		for (const auto& message : messages)
		{
			objs.append(toJson(message));
		}

		QJsonDocument doc(objs);

		std::ofstream out(file);
		if (!out.is_open())
		{
			return false;
		}

		out << doc.toJson().toStdString();
		out.close();

		return true;
	}

	QJsonObject Export::toJson(const LogObject::Info& info)
	{
		QJsonObject obj;
		obj["id"] = (int)info.id;
		obj["parentId"] = (int)info.parentId;
		obj["name"] = info.name.c_str();
		obj["creationTime"] = info.creationTime.toString(Log::DateTime::Format::yearMonthDay | Log::DateTime::Format::hourMinuteSecondMillisecond).c_str();
		obj["color"] = info.color.getRGBStr().c_str();
		obj["enabled"] = info.enabled;
		return obj;
	
	}
	QJsonObject Export::toJson(const Message& message)
	{
		QJsonObject obj;
		obj["id"] = (int)message.getLoggerID();
		obj["level"] = message.getLevel();
		obj["text"] = message.getText().c_str();
		obj["color"] = message.getColor().getRGBStr().c_str();
		obj["dateTime"] = message.getDateTime().toString(Log::DateTime::Format::yearMonthDay | Log::DateTime::Format::hourMinuteSecondMillisecond).c_str();
		return obj;
	}
	QJsonObject Export::getLibraryInfo()
	{
		QJsonObject obj;
		obj["name"] = LibraryInfo::name;
		obj["version"] = LibraryInfo::version.toString().c_str();
		return obj;
	}
	QJsonObject Export::getLogLevelInfo()
	{
		QJsonObject obj;
		for (int i = 0; i < (int)Level::__count; ++i)
			obj[Utilities::getLevelStr((Level)i).c_str()] = i;
		return obj;
	}
	QJsonObject Export::getFileHeader()
	{
		QJsonObject libraryInfo = getLibraryInfo();
		libraryInfo["levelInfo"] = getLogLevelInfo();
		return libraryInfo;
	}
} 