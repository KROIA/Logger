#include "LogMessage.h"

namespace Log
{

	LevelColors Message::s_levelColors = {
		Color::cyan,		// trace,
		Color::magenta,		// debug,
		Color::white,		// info,
		Color::yellow,		// warning,
		Color::red,			// error,
		Color::green		// custom
	};


	

	Message::Message(const std::string& msg)
		: m_message(msg)
		, m_level(Level::info)
		, m_customColor(Color::white)
		, m_useCustomColor(false)
	{
		//m_color = getLevelColor(m_level);
	}
	Message::Message(const std::string& msg, Level level)
		: m_message(msg)
		, m_level(level)
		, m_customColor(Color::white)
		, m_useCustomColor(false)
	{
		//m_color = getLevelColor(m_level);
	}
	Message::Message(const std::string& msg, Level level, const Color& col)
		: m_message(msg)
		, m_level(level)
		, m_customColor(col)
		, m_useCustomColor(true)
	{

	}
	Message::Message(const char* msg)
		: m_message(msg)
		, m_level(Level::info)
		, m_customColor(Color::white)
		, m_useCustomColor(false)
		//, m_color(Color::white)
		//, m_autoSetColor(true)
	{
		//m_color = getLevelColor(m_level);
	}
	Message::Message(const char* msg, Level level)
		: m_message(msg)
		, m_level(level)
		, m_customColor(Color::white)
		, m_useCustomColor(false)
		//, m_color(Color::white)
		//, m_autoSetColor(true)
	{
		//m_color = getLevelColor(m_level);
	}
	Message::Message(const char* msg, Level level, const Color& col)
		: m_message(msg)
		, m_level(level)
		, m_customColor(col)
		, m_useCustomColor(true)
	{

	}

	Message::Message(const Message& other)
		: m_message(other.m_message)
		, m_level(other.m_level)
		, m_customColor(other.m_customColor)
		, m_useCustomColor(other.m_useCustomColor)
	{

	}

	Message::~Message()
	{

	}

	Message& Message::operator=(const Message& other)
	{
		m_message = other.m_message;
		//m_color = other.m_color;
		m_level = other.m_level;
		//m_autoSetColor = other.m_autoSetColor;
		return *this;
	}
	Message Message::operator+(const Message& other) const
	{
		return Message(m_message + other.m_message, m_level/*, m_color*/);
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
		if (m_useCustomColor != other.m_useCustomColor)
			return false;
		if (m_customColor != other.m_customColor)
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
		if (m_useCustomColor != other.m_useCustomColor)
			return true;
		if (m_customColor != other.m_customColor)
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
		m_customColor = color;
	}
	const Color& Message::getColor() const
	{
		if (m_level != Level::custom)
			return getLevelColor(m_level);
		return m_customColor;
	}

	void Message::setLevel(Level level)
	{
		m_level = level;
		if (m_level != Level::custom)
		{
			m_useCustomColor = false;
			m_customColor = Color::white;
		}
		else
		{
			if (!m_useCustomColor)
			{
				m_useCustomColor = true;
				m_customColor = getLevelColor(m_level);
			}
		}
		//if (m_autoSetColor)
			//m_color = getLevelColor(m_level);
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

	const Color& Message::getLevelColor(Level l)
	{
		switch (l)
		{
			case Level::trace:    return s_levelColors.trace;
			case Level::debug:    return s_levelColors.debug;
			case Level::info:     return s_levelColors.info;
			case Level::warning:  return s_levelColors.warning;
			case Level::error:    return s_levelColors.error;
			case Level::custom:   return s_levelColors.custom;
		}
		return Color::white;
	}
	const LevelColors& Message::getLevelColors()
	{
		return s_levelColors;
	}
	void Message::setLevelColors(const LevelColors& colors)
	{
		s_levelColors = colors;
	}

	/*void Message::autoSetColor(bool enable)
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
	}*/
}