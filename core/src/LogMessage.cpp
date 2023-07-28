#include "LogMessage.h"

namespace Log
{

	Message::Message(const std::string& msg)
		: m_message(msg)
		, m_level(Level::info)
		, m_color(Color::white)
		, m_autoSetColor(true)
	{
		m_color = getLevelColor(m_level);
	}
	Message::Message(const std::string& msg, Level level)
		: m_message(msg)
		, m_level(level)
		, m_color(Color::white)
		, m_autoSetColor(true)
	{
		m_color = getLevelColor(m_level);
	}
	Message::Message(const std::string& msg, Level level, const Color& col)
		: m_message(msg)
		, m_level(level)
		, m_color(col)
		, m_autoSetColor(false)
	{

	}
	Message::Message(const char* msg)
		: m_message(msg)
		, m_level(Level::info)
		, m_color(Color::white)
		, m_autoSetColor(true)
	{
		m_color = getLevelColor(m_level);
	}
	Message::Message(const char* msg, Level level)
		: m_message(msg)
		, m_level(level)
		, m_color(Color::white)
		, m_autoSetColor(true)
	{
		m_color = getLevelColor(m_level);
	}
	Message::Message(const char* msg, Level level, const Color& col)
		: m_message(msg)
		, m_level(level)
		, m_color(col)
		, m_autoSetColor(false)
	{

	}

	Message::Message(const Message& other)
		: m_message(other.m_message)
		, m_level(other.m_level)
		, m_color(other.m_color)
		, m_autoSetColor(other.m_autoSetColor)
	{

	}

	Message::~Message()
	{

	}

	Message& Message::operator=(const Message& other)
	{
		m_message = other.m_message;
		m_color = other.m_color;
		m_level = other.m_level;
		m_autoSetColor = other.m_autoSetColor;
		return *this;
	}
	Message Message::operator+(const Message& other) const
	{
		return Message(m_message + other.m_message, m_level, m_color);
	}
	Message& Message::operator+=(const Message& other)
	{
		m_message += other.m_message;
		return *this;
	}

	bool Message::operator==(const Message& other) const
	{
		if (m_level != other.m_level)
			return false;
		if (m_color != other.m_color)
			return false;
		if (m_message.size() != other.m_message.size())
			return false;
		if (m_message == other.m_message)
			return true;
		return false;
	}
	bool Message::operator!=(const Message& other) const
	{
		if (m_level != other.m_level)
			return true;
		if (m_color != other.m_color)
			return true;
		if (m_message.size() != other.m_message.size())
			return true;
		if (m_message == other.m_message)
			return false;
		return true;
	}

	void Message::setText(const std::string& text)
	{
		m_message = text;
	}
	void Message::setText(const char* text)
	{
		m_message.assign(text);
	}
	const std::string& Message::getText() const
	{
		return m_message;
	}

	void Message::setColor(const Color& color)
	{
		m_color = color;
	}
	const Color& Message::getColor() const
	{
		return m_color;
	}

	void Message::setLevel(Level level)
	{
		m_level = level;
		if (m_autoSetColor)
			m_color = getLevelColor(m_level);
	}
	Level Message::getLevel() const
	{
		return m_level;
	}
	void Message::updateTimestamp()
	{
		m_dateTime.update();
	}
	const DateTime& Message::getDateTime() const
	{
		return m_dateTime;
	}

	void Message::autoSetColor(bool enable)
	{
		m_autoSetColor = enable;
		if(m_autoSetColor)
			m_color = getLevelColor(m_level);
	}
	Color Message::getLevelColor(Level level)
	{
		switch (level)
		{
			case Level::trace:   return Color::cyan;
			case Level::debug:   return Color::magenta;
			case Level::info:    return Color::white;
			case Level::warning: return Color::yellow;
			case Level::error:   return Color::red;
		}
		return Color::white;
	}
}