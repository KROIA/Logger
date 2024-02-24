#pragma once

#include "Logger_base.h"
#include "LogColor.h"
#include "LogLevel.h"
#include "Utilities/DateTime.h"
#include <string>
#include <vector>

namespace Log
{
	namespace Logger
	{
		class AbstractLogger;
	}
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

		Message& operator=(const std::string& text);
		Message& operator=(const Message& other);
		Message operator+(const Message& other) const;
		Message& operator+=(const Message& other);

		bool operator==(const Message& other) const;
		bool operator!=(const Message& other) const;

		void setText(const std::string& text);
		void setText(const char* text);
		const std::string& getText() const;
		
		void setContext(Logger::AbstractLogger* context);
		Logger::AbstractLogger* getContext() const;

		void setColor(const Color& color);
		const Color& getColor() const;

		void setLevel(Level level);
		Level getLevel() const;
		std::string getLevelString() const;

		void updateTimestamp();
		const DateTime& getDateTime() const;

		void setTabCount(unsigned int count);
		unsigned int getTabCount() const;

		std::string toString() const;

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
		Logger::AbstractLogger* m_context;
		Level m_level;
		Color m_customColor;
		bool m_useCustomColor;
		unsigned int m_tabCount;

		DateTime m_dateTime;
		
	private:

		static LevelColors s_levelColors; // Initialized in LogColor.cpp
	};
}