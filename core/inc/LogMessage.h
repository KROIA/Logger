#pragma once

#include "Logger_base.h"
#include "LogColor.h"
#include "LogLevel.h"
#include "Utilities/DateTime.h"
#include <string>
#include <vector>


namespace Log
{
	typedef unsigned int LoggerID;

	class LOGGER_EXPORT Message
	{
	public:
		Message();
		explicit Message(const std::string& msg);
		explicit Message(const std::string& msg, Level level);
		explicit Message(const std::string& msg, Level level, const Color& col);

		explicit Message(const char* msg);
		explicit Message(const char* msg, Level level);
		explicit Message(const char* msg, Level level, const Color& col);

		Message(const Message& other);

		~Message();

		Message& operator=(const std::string& text);
		Message& operator=(const Message& other);
		Message operator+(const Message& other) const;
		Message& operator+=(const Message& other);

		bool operator==(const Message& other) const;
		bool operator!=(const Message& other) const;

		void setText(const std::string& text);
		void setText(const char* text);
		const std::string& getText() const;

		void setColor(const Color& color);
		const Color& getColor() const;

		void setLevel(Level level);
		Level getLevel() const;
		std::string getLevelString() const;

		void updateTimestamp();
		const DateTime& getDateTime() const;

		void setLoggerID(LoggerID id){ m_loggerID = id; }
		LoggerID getLoggerID() const { return m_loggerID; }

		std::string toString(DateTime::Format format) const;

		static const Color& getLevelColor(Level l);
		static const LevelColors& getLevelColors();
		static void setLevelColors(const LevelColors& colors);

		enum SortType
		{
			timeAscending,
			timeDescending
		};
		static void sort(std::vector<Message>& messages, SortType type);


	protected:
		std::string m_message;
		Level m_level;
		Color m_customColor;
		bool m_useCustomColor;

		DateTime m_dateTime;
		LoggerID m_loggerID;
		
	private:
		void normalizeMessage();

		static LevelColors s_levelColors; 

		
	};
}

Q_DECLARE_METATYPE(Log::Message);