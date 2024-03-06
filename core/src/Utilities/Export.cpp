#include "Utilities/Export.h"
#include <fstream>

#include "LibraryInfo.h"

namespace Log
{
	bool Export::saveToFile(const std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list, const std::string& file)
	{
		Log::DateTime::Format timeFormat = Log::DateTime::Format::yearMonthDay | Log::DateTime::Format::hourMinuteSecondMillisecond;
		bool compact = false;

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


		for (size_t i = 0; i < list.size(); i++)
		{
			const Logger::AbstractLogger::LoggerSnapshotData &data = list[i];
			const Logger::AbstractLogger::LoggerMetaInfo &metaInfo = data.metaInfo;
			const std::vector<Message::SnapshotData> &messages = data.messages;


			std::string contextInfo =
				"Context[" + std::to_string(metaInfo.id) + "]{\n" +
				"\tname=" + metaInfo.name + "\n" +
				"\tcreationTime=" + metaInfo.creationTime.toString(timeFormat) + "\n" +
				"\tparentID=" + std::to_string(metaInfo.parentId) + "\n" +
				"\tcolor=" + metaInfo.color.getRGBStr() + "\n" +
				"\tenabled=" + std::to_string(metaInfo.enabled) + "\n" +
				"\ttabCount=" + std::to_string(metaInfo.tabCount) + "\n"+
				"\tmessages={\n";

			out << contextInfo;

			for (size_t j = 0; j < messages.size(); j++)
			{
				out << "\t[" << j << "]{";

				out << "Level=" << messages[j].level << std::endl;

				std::string messageText = messages[j].message;

				// Replace newlines with \n and " with \"
				for (size_t k = 0; k < messageText.size(); k++)
				{
					if (messageText[k] == '\n')
					{
						messageText.replace(k, 1, "\\n");
					}
					else if (messageText[k] == '\"')
					{
						messageText.replace(k, 1, "\\\"");
					}
				}
				out << "\t\tMessage=\"" << messageText <<"\"" << std::endl;
				out << "\t\tColor=" << messages[j].textColor.getRGBStr() << std::endl;
				out << "\t\tDateTime=\"" << messages[j].dateTime.toString(timeFormat)<<"\"" << std::endl;
				out << "\t\tTabCount=" << messages[j].tabCount << "\t}\n";
			}
			out << "\t}\n}\n";
		}
		out.close();
		return true;
	}

} 