#pragma once

#include "Logger_base.h"
#include "LogColor.h"
#include "Utilities/DateTime.h"
#include "LogMessage.h"

namespace Log
{
	class LOGGER_EXPORT LogObject
	{
	public:
		
		struct Info
		{
			LoggerID id;
			LoggerID parentId;
			//std::vector<LoggerID> childrenIds;
			std::string name;
			DateTime creationTime;
			Color color;
			bool enabled;

			Info()
				: id(0)
				, parentId(0)
				, name("")
				, creationTime(DateTime())
				, color(Colors::white)
				, enabled(false)
			{}
			Info(LoggerID id,
				LoggerID parentID,
				const std::string& name,
				const DateTime& creationTime,
				const Color& color,
				bool enabled)
				: id(id)
				, parentId(parentID)
				, name(name)
				, creationTime(creationTime)
				, color(color)
				, enabled(enabled)
			{}
			Info(const Info& other)
				: id(other.id)
				, parentId(other.parentId)
				//, childrenIds(other.childrenIds)
				, name(other.name)
				, creationTime(other.creationTime)
				, color(other.color)
				, enabled(other.enabled)
			{}

			std::string toString() const
			{
				std::string str = "ID: " + std::to_string(id) + "\n";
				str += "Parent ID: " + std::to_string(parentId) + "\n";
				str += "Name: " + name + "\n";
				str += "Creation Time: " + creationTime.toString(Log::DateTime::Format::dayMonthYear | Log::DateTime::Format::hourMinuteSecondMillisecond) + "\n";
				str += "Color: " + color.getRGBStr() + "\n";
				str += "Enabled: " + std::to_string(enabled) + "\n";
				return str;
			}
		};

		

		LogObject(const std::string& name = "LogObject");
		LogObject(const LogObject& other, const std::string& name = "LogObject");
		LogObject(LoggerID parentID, const std::string& name = "LogObject");

		~LogObject();

		void setEnabled(bool enable);
		bool isEnabled() const;
		void setName(const std::string& name);
		std::string getName() const;
		void setColor(const Color& col);
		Color getColor() const;
		DateTime getCreationDateTime() const;
		LoggerID getID() const;
		void setParentID(LoggerID parentID);
		LoggerID getParentID() const;



		void log(const Message &msg);

		void log(const std::string& msg);
		void log(const std::string& msg, Level level);
		void log(const std::string& msg, Level level, const Color& col);

		void logTrace(const std::string& msg);
		void trace(const std::string& msg) { logTrace(msg); }
		void logDebug(const std::string& msg);
		void debug(const std::string& msg) { logDebug(msg); }
		void logInfo(const std::string& msg);
		void info(const std::string& msg) { logInfo(msg); }
		void logWarning(const std::string& msg);
		void warning(const std::string& msg) { logWarning(msg); }
		void logError(const std::string& msg);
		void error(const std::string& msg) { logError(msg); }
		void logCustom(const std::string& msg);
		void custom(const std::string& msg) { logCustom(msg); }

		static void setLevelEnabled(Level level, bool enabled);

	protected:


	private:
		LoggerID m_id;
	};
}

Q_DECLARE_METATYPE(Log::LogObject::Info);