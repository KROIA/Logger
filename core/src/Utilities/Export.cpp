#include "Utilities/Export.h"
#include <fstream>

#include "Logger_info.h"
#include "LogManager.h"
#include "LogObject.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace Log
{
	bool Export::saveToFile(const std::unordered_map<LoggerID, std::vector<Message>>& contexts, const std::string& file)
	{
		Log::DateTime::Format timeFormat = Log::DateTime::Format::yearMonthDay | Log::DateTime::Format::hourMinuteSecondMillisecond;

		QJsonObject root;

		QJsonObject libraryInfo;
		libraryInfo["name"] = LibraryInfo::name;
		libraryInfo["version"] = LibraryInfo::versionStr().c_str();

		QJsonObject levelInfo;
		for(int i=0; i<(int)Level::__count; ++i)
			levelInfo[Utilities::getLevelStr((Level)i).c_str()] = i;
		libraryInfo["levelInfo"] = levelInfo;

		root["libraryInfo"] = libraryInfo;

		QJsonArray contextArray;
		for (const auto& context : contexts)
		{
			QJsonObject contextObject;
			const LogObject::Info &metaInfo = LogManager::getLogObjectInfo(context.first);

			contextObject["id"] = (int)context.first;
			contextObject["name"] = metaInfo.name.c_str();
			contextObject["creationTime"] = metaInfo.creationTime.toString(timeFormat).c_str();
			contextObject["parentId"] = (int)metaInfo.parentId;
			contextObject["color"] = metaInfo.color.getRGBStr().c_str();
			//contextObject["enabled"] = metaInfo.enabled;

			QJsonArray messageArray;
			for (const auto& message : context.second)
			{
				QJsonObject messageObject;
				messageObject["level"] = message.getLevel();
				messageObject["text"] = message.getText().c_str();
				messageObject["color"] = message.getColor().getRGBStr().c_str();
				messageObject["dateTime"] = message.getDateTime().toString(timeFormat).c_str();

				messageArray.append(messageObject);
			}
			contextObject["messages"] = messageArray;
			contextArray.append(contextObject);
		}
		root["contexts"] = contextArray;

		QJsonDocument doc(root);

		std::ofstream out(file);
		if (!out.is_open())
		{
			return false;
		}

		out << doc.toJson().toStdString();
		out.close();

		return true;
	}

} 