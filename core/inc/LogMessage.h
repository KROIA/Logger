#pragma once

#include "Logger_base.h"
#include "LogColor.h"
#include "LogLevel.h"
#include "DateTime.h"
#include <string>
#include <vector>

namespace Log
{
	class LOGGER_EXPORT Message
	{
	public:
		Message(const std::string& msg);
		Message(const std::string& msg, Level level);
		Message(const std::string& msg, Level level, const Color& col);

		Message(const char* msg);
		Message(const char* msg, Level level);
		Message(const char* msg, Level level, const Color& col);

		Message(const Message& other);

		~Message();

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

		void updateTimestamp();
		const DateTime& getDateTime() const;

		//void autoSetColor(bool enable);
		//static Color getLevelColor(Level level);


		static const Color& getLevelColor(Level l);
		static const LevelColors& getLevelColors();
		static void setLevelColors(const LevelColors& colors);

	protected:
		std::string m_message;
		Level m_level;
		Color m_customColor;
		bool m_useCustomColor;

		//bool m_autoSetColor;
		DateTime m_dateTime;
		
	private:

		static LevelColors s_levelColors; // Initialized in LogColor.cpp
	};

	/*struct LOGGER_EXPORT NastedMessage
	{
		Message message;
		std::vector<NastedMessage> childMessages;
	};*/
}