#include "LogMessage.h"
#include "LoggerTypes/AbstractLogger.h"
#include <algorithm>
namespace Log
{
	Message::Message(const std::string& msg)
		: m_message(msg)
		, m_context(nullptr)
		, m_level(Level::info)
		, m_customColor(Color::white)
		, m_useCustomColor(false)
		, m_tabCount(0)
	{

	}
	Message::Message(const std::string& msg, Level level)
		: m_message(msg)
		, m_context(nullptr)
		, m_level(level)
		, m_customColor(Color::white)
		, m_useCustomColor(false)
		, m_tabCount(0)
	{

	}
	Message::Message(const std::string& msg, Level level, const Color& col)
		: m_message(msg)
		, m_context(nullptr)
		, m_level(level)
		, m_customColor(col)
		, m_useCustomColor(true)
		, m_tabCount(0)
	{

	}
	Message::Message(const char* msg)
		: m_message(msg)
		, m_context(nullptr)
		, m_level(Level::info)
		, m_customColor(Color::white)
		, m_useCustomColor(false)
		, m_tabCount(0)
	{

	}
	Message::Message(const char* msg, Level level)
		: m_message(msg)
		, m_context(nullptr)
		, m_level(level)
		, m_customColor(Color::white)
		, m_useCustomColor(false)
		, m_tabCount(0)
	{

	}
	Message::Message(const char* msg, Level level, const Color& col)
		: m_message(msg)
		, m_context(nullptr)
		, m_level(level)
		, m_customColor(col)
		, m_useCustomColor(true)
		, m_tabCount(0)
	{

	}

	Message::Message(const Message& other)
		: m_message(other.m_message)
		, m_context(other.m_context)
		, m_level(other.m_level)
		, m_customColor(other.m_customColor)
		, m_useCustomColor(other.m_useCustomColor)
		, m_tabCount(other.m_tabCount)
		, m_dateTime(other.m_dateTime)
	{

	}

	Message::~Message()
	{

	}
	Message& Message::operator=(const std::string& text)
	{
		m_message = text;
		return *this;
	}
	Message& Message::operator=(const Message& other)
	{
		m_message = other.m_message;
		m_context = other.m_context;
		m_level = other.m_level;
		m_customColor = other.m_customColor;
		m_useCustomColor = other.m_useCustomColor;
		m_tabCount = other.m_tabCount;
		m_dateTime = other.m_dateTime;
		return *this;
	}
	Message Message::operator+(const Message& other) const
	{
		return Message(m_message + other.m_message, m_level);
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

	void Message::setContext(Logger::AbstractLogger* context)
	{
		m_context = context;
	}
	Logger::AbstractLogger* Message::getContext() const
	{
		return m_context;
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

	void Message::setTabCount(unsigned int count)
	{
		m_tabCount = count;
	}
	unsigned int Message::getTabCount() const
	{
		return m_tabCount;
	}

	std::string Message::toString(DateTime::Format format) const
	{
		return m_dateTime.toString(format) + " " +
			Utilities::getLevelStr(m_level) +
			std::string(m_tabCount, ' ') +
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

	Message::SnapshotData Message::createSnapshot() const
	{
		SnapshotData data;
		data.message = m_message;
		if (m_context)
		{
			data.contextName = m_context ? m_context->getName() : "";
			data.loggerID = m_context->getID();
			data.contextColor = m_context->getColor();
		}
		else
		{
			data.loggerID = 0;
			data.contextColor = Color::white;
		}
		data.level = m_level;
		data.textColor = getColor();
		
		data.tabCount = m_tabCount;
		data.dateTime = m_dateTime;
		return data;
	}
}