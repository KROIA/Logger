#include "LogMessage.h"
#include <algorithm>

namespace Log
{

	LevelColors Message::s_levelColors{
		Colors::cyan,		// trace,
		Colors::magenta,	// debug,
		Colors::white,		// info,
		Colors::yellow,		// warning,
		Colors::red,		// error,
		Colors::green		// custom
	};

	Message::Message()
		: m_message("")
		, m_level(Level::info)
		, m_customColor(Colors::white)
		, m_useCustomColor(false)
		, m_loggerID(0)
	{

	}
	Message::Message(const std::string& msg)
		: m_message(msg)
		, m_level(Level::info)
		, m_customColor(Colors::white)
		, m_useCustomColor(false)
		, m_loggerID(0)
	{
		normalizeMessage();
	}
	Message::Message(const std::string& msg, Level level)
		: m_message(msg)
		, m_level(level)
		, m_customColor(Colors::white)
		, m_useCustomColor(false)
		, m_loggerID(0)
	{
		normalizeMessage();
	}
	Message::Message(const std::string& msg, Level level, const Color& col)
		: m_message(msg)
		, m_level(level)
		, m_customColor(col)
		, m_useCustomColor(true)
		, m_loggerID(0)
	{
		normalizeMessage();
	}
	Message::Message(const char* msg)
		: m_message(msg)
		, m_level(Level::info)
		, m_customColor(Colors::white)
		, m_useCustomColor(false)
		, m_loggerID(0)
	{
		normalizeMessage();
	}
	Message::Message(const char* msg, Level level)
		: m_message(msg)
		, m_level(level)
		, m_customColor(Colors::white)
		, m_useCustomColor(false)
		, m_loggerID(0)
	{
		normalizeMessage();
	}
	Message::Message(const char* msg, Level level, const Color& col)
		: m_message(msg)
		, m_level(level)
		, m_customColor(col)
		, m_useCustomColor(true)
		, m_loggerID(0)
	{
		normalizeMessage();
	}

	Message::Message(const Message& other)
		: m_message(other.m_message)
		, m_level(other.m_level)
		, m_customColor(other.m_customColor)
		, m_useCustomColor(other.m_useCustomColor)
		, m_dateTime(other.m_dateTime)
		, m_loggerID(other.m_loggerID)
	{

	}

	Message::~Message()
	{

	}
	Message& Message::operator=(const std::string& text)
	{
		m_message = text;
		normalizeMessage();
		return *this;
	}
	Message& Message::operator=(const Message& other)
	{
		m_message = other.m_message;
		m_level = other.m_level;
		m_customColor = other.m_customColor;
		m_useCustomColor = other.m_useCustomColor;
		m_dateTime = other.m_dateTime;
		m_loggerID = other.m_loggerID;
		return *this;
	}
	Message Message::operator+(const Message& other) const
	{
		return Message(m_message + other.m_message, m_level);
	}
	Message& Message::operator+=(const Message& other)
	{
		m_message += other.m_message;
		normalizeMessage();
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
		if(m_loggerID != other.m_loggerID)
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
		if (m_loggerID != other.m_loggerID)
			return true;
		if (m_message == other.m_message)
			return false;
		return true;
	}

	void Message::setText(const std::string& text)
	{
		m_message = text;
		normalizeMessage();
	}
	void Message::setText(const char* text)
	{
		m_message = std::string(text);
		normalizeMessage();
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
		if (m_level != Level::custom && !m_useCustomColor)
			return getLevelColor(m_level);
		return m_customColor;
	}

	void Message::setLevel(Level level)
	{
		m_level = level;
		if (m_level != Level::custom)
		{
			m_useCustomColor = false;
			m_customColor = Colors::white;
		}
		else
		{
			if (!m_useCustomColor)
			{
				m_useCustomColor = true;
				m_customColor = getLevelColor(m_level);
			}
		}
	}
	Level Message::getLevel() const
	{
		return m_level;
	}
	std::string Message::getLevelString() const
	{
		return Utilities::getLevelStr(m_level);
	}
	void Message::updateTimestamp()
	{
		m_dateTime.update();
	}
	const DateTime& Message::getDateTime() const
	{
		return m_dateTime;
	}

	std::string Message::toString(DateTime::Format format) const
	{
		return m_dateTime.toString(format) + " " +
			Utilities::getLevelStr(m_level) +
			m_message;
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
		return Colors::white;
	}
	const LevelColors& Message::getLevelColors()
	{
		return s_levelColors;
	}
	void Message::setLevelColors(const LevelColors& colors)
	{
		s_levelColors = colors;
	}

	void Message::sort(std::vector<Message>& messages, SortType type)
	{
		switch (type)
		{
			case SortType::timeAscending:
				std::sort(messages.begin(), messages.end(), [](const Message& a, const Message& b) 
					{ 
						return a.m_dateTime.getTime() < b.m_dateTime.getTime();
					});
				break;
			case SortType::timeDescending:
				std::sort(messages.begin(), messages.end(), [](const Message& a, const Message& b) { return a.m_dateTime > b.m_dateTime; });
				break;
		}
	}

	void Message::normalizeMessage()
	{
		while ( m_message.find_last_of("\n") == m_message.size() - 1 && 
				m_message.size() > 0)
		{
			m_message.erase(m_message.size() - 1, 1);
		}
	}
}