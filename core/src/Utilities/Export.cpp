#include "Utilities/Export.h"
#include <fstream>

#include "Logger_info.h"
#include "LogManager.h"
#include "LogObject.h"

namespace Log
{
	bool Export::saveToFile(const std::unordered_map<LoggerID, std::vector<Message>>& contexts, const std::string& file)
	{
		Log::DateTime::Format timeFormat = Log::DateTime::Format::yearMonthDay | Log::DateTime::Format::hourMinuteSecondMillisecond;

		std::ofstream out(file);
		if (!out.is_open())
		{
			return false;
		}

		static const std::string libInfo = 
			std::string("# Saved by library: \"") + LibraryInfo::name + "\"" +
			"\tVersion: " + LibraryInfo::versionStr() + 
			"\tBuildtype: " + LibraryInfo::buildTypeStr + "\n";

		out << libInfo;


		for (const auto listIt : contexts)
		{
			//const Logger::AbstractLogger::LoggerSnapshotData &data = list[i];
			//const Logger::AbstractLogger::MetaInfo &metaInfo = data.metaInfo;
			const LoggerID id = listIt.first;
			const std::vector<Message>& messages = listIt.second;
			const LogObject::Info &metaInfo = LogManager::getLogObjectInfo(id);

			std::string contextInfo =
				"Context[" + std::to_string(id) + "]{\n" +
				"\tname=" + metaInfo.name + "\n" +
				"\tcreationTime=" + metaInfo.creationTime.toString(timeFormat) + "\n" +
				"\tparentID=" + std::to_string(metaInfo.parentId) + "\n" +
				"\tcolor=" + metaInfo.color.getRGBStr() + "\n" +
				"\tenabled=" + std::to_string(metaInfo.enabled) + "\n" +
				"\tmessages={\n";

			out << contextInfo;

			for (size_t j = 0; j < messages.size(); j++)
			{
				out << "\t[" << j << "]{";

				out << "Level=" << messages[j].getLevel() << std::endl;

				std::string messageText = messages[j].getText();

				// Replace newlines with \n and " with \"
				for (size_t k = 0; k < messageText.size(); k++)
				{
					if (messageText[k] == '\n')
					{
						messageText.replace(k, 1, "\\n");
						k++;
					}
					else if (messageText[k] == '\"')
					{
						messageText.replace(k, 1, "\\\"");
						k++;
					}
				}
				out << "\t\tMessage=\"" << messageText <<"\"" << std::endl;
				out << "\t\tColor=" << messages[j].getColor().getRGBStr() << std::endl;
				out << "\t\tDateTime=\"" << messages[j].getDateTime().toString(timeFormat) << "\"\t}\n";
			}
			out << "\t}\n}\n";
		}
		out.close();
		return true;
	}

} 