#pragma once

#include "Logger_base.h"
#include "LogColor.h"
#include "LogLevel.h"
#include "DateTime.h"
#include <string>

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

		void autoSetColor(bool enable);
		static Color getLevelColor(Level level);

	protected:
		std::string m_message;
		Level m_level;
		Color m_color;
		bool m_autoSetColor;
		DateTime m_dateTime;
		
	private:
		
	};
}