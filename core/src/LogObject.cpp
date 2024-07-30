#include "LogObject.h"
#include "LogManager.h"

namespace Log
{
	LogObject::LogObject(const std::string& name)
	{
		Info info(0, 0, name, DateTime(), Color::white, true);
		m_id = LogManager::addNewLogger(info);
	}
	LogObject::LogObject(const LogObject& other, const std::string& name)
	{
		Info info(0, other.getID(), name, DateTime(), Color::white, true);
		m_id = LogManager::addNewLogger(info);
	}
	LogObject::LogObject(LoggerID parentID, const std::string& name)
	{
		Info info(0, parentID, name, DateTime(), Color::white, true);
		m_id = LogManager::addNewLogger(info);
	}

	LogObject::~LogObject()
	{

	}

	void LogObject::setEnabled(bool enable)
	{
		Info info = LogManager::getLogObjectInfo(m_id);
		info.enabled = enable;
		LogManager::setLogObjectInfo(info);
	}
	bool LogObject::isEnabled() const
	{
		return LogManager::getLogObjectInfo(m_id).enabled;
	}

	void LogObject::setName(const std::string& name)
	{
		Info info = LogManager::getLogObjectInfo(m_id);
		info.name = name;
		LogManager::setLogObjectInfo(info);
	}
	std::string LogObject::getName() const
	{
		return LogManager::getLogObjectInfo(m_id).name;
	}

	void LogObject::setColor(const Color& col)
	{
		Info info = LogManager::getLogObjectInfo(m_id);
		info.color = col;
		LogManager::setLogObjectInfo(info);
	}
	Color LogObject::getColor() const
	{
		return LogManager::getLogObjectInfo(m_id).color;
	}

	DateTime LogObject::getCreationDateTime() const
	{
		return LogManager::getLogObjectInfo(m_id).creationTime;
	}
	LoggerID LogObject::getID() const
	{
		return m_id;
	}
	void LogObject::setParentID(LoggerID parentID)
	{
		LogManager::onChangeParentInternal(m_id, parentID);
	}
	LoggerID LogObject::getParentID() const
	{
		return LogManager::getLogObjectInfo(m_id).parentId;
	}



	void LogObject::log(const Message &msg)
	{
		Message temp = msg;
		temp.setLoggerID(m_id);
		LogManager::onLogMessageInternal(temp);
	}

	void LogObject::log(const std::string& msg)
	{
		log(msg, Level::info);
	}
	void LogObject::log(const std::string& msg, Level level)
	{
		Message m(msg, level);
		m.setLoggerID(m_id);
		LogManager::onLogMessageInternal(m);
	}
	void LogObject::log(const std::string& msg, Level level, const Color& col)
	{
		Message m(msg, level, col);
		m.setLoggerID(m_id);
		LogManager::onLogMessageInternal(m);
	}

	void LogObject::logTrace(const std::string& msg)
	{
		log(msg, Level::trace);
	}
	void LogObject::logDebug(const std::string& msg)
	{
		log(msg, Level::debug);
	}
	void LogObject::logInfo(const std::string& msg)
	{
		log(msg, Level::info);
	}
	void LogObject::logWarning(const std::string& msg)
	{
		log(msg, Level::warning);
	}
	void LogObject::logError(const std::string& msg)
	{
		log(msg, Level::error);
	}
	void LogObject::logCustom(const std::string& msg)
	{
		log(msg, Level::custom);
	}
}
