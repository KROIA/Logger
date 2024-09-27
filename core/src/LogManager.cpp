#include "LogManager.h"

namespace Log
{
	LogManager::LogManager()
		: QObject()
	{
		m_nextID = 1;

		for (size_t i = 0; i < static_cast<size_t>(Level::__count); i++)
		{
			m_enabledLevels[i] = true;
		}
	}

	LoggerID LogManager::addNewLogger(LogObject::Info loggerInfo)
	{
		LogManager &m = instance();
		{
			const std::lock_guard<std::mutex> lock(m.m_mutex);

			// Check if the ID is already in use
			while (m.m_logObjects.find(m.m_nextID) != m.m_logObjects.end())
			{
				m.m_nextID++;
			}

			loggerInfo.id = m.m_nextID;
			m.m_logObjects[loggerInfo.id] = loggerInfo;
		}
		emit m.onNewLogger(loggerInfo);
		return loggerInfo.id;
	}
	void LogManager::onLogMessageInternal(Message message)
	{
		LogManager& m = instance();

		// If the level is not enabled, do not emit the signal
		if (!m.m_enabledLevels[static_cast<size_t>(message.getLevel())])
			return;

		bool emitSignal = false;
		{
			const std::lock_guard<std::mutex> lock(m.m_mutex);
			const auto& it = m.m_logObjects.find(message.getLoggerID());
			if (it != m.m_logObjects.end())
			{
				if (it->second.enabled)
				{
					emitSignal = true;
				}
			}
		}
		if (emitSignal)
		{
			emit m.onLogMessage(message);
		}
	}
	void LogManager::onChangeParentInternal(LoggerID childID, LoggerID newParentID)
	{
		LogManager& m = instance();
		bool emitSignal = false;
		{
			const std::lock_guard<std::mutex> lock(m.m_mutex);
			const auto& it = m.m_logObjects.find(childID);
			if (it != m.m_logObjects.end())
			{
				it->second.parentId = newParentID;
				emitSignal = true;
			}
		}
		if (emitSignal)
		{
			emit m.onChangeParent(childID, newParentID);
		}
	}

	void LogManager::setLogObjectInfo(LogObject::Info info)
	{
		bool emitSignal = false;
		LogManager& m = instance();
		{
			const std::lock_guard<std::mutex> lock(m.m_mutex);
			const auto& it = m.m_logObjects.find(info.id);
			if (it != m.m_logObjects.end())
			{
				it->second = info;
				emitSignal = true;
			}
		}		
		if (emitSignal)
		{
			emit m.onLoggerInfoChanged(info);
		}
	}

	LogManager& LogManager::instance()
	{
		static LogManager instance;
		return instance;
	}

	LogObject::Info LogManager::getLogObjectInfo(LoggerID loggerID)
	{
		LogManager& m = instance();
		{
			const std::lock_guard<std::mutex> lock(m.m_mutex);
			const auto& it = m.m_logObjects.find(loggerID);
			if (it != m.m_logObjects.end())
				return it->second;
		}
		static LogObject::Info dummyInfo;
		return dummyInfo;
	}
	std::vector<LogObject::Info> LogManager::getLogObjectsInfo()
	{
		std::vector<LogObject::Info> result;
		LogManager& m = instance();
		{
			const std::lock_guard<std::mutex> lock(m.m_mutex);
			for (const auto& it : m.m_logObjects)
			{
				result.push_back(it.second);
			}
		}
		return result;
	}

	void LogManager::setLevelEnabled(Level level, bool enabled)
	{
		LogManager& m = instance();
		{
			const std::lock_guard<std::mutex> lock(m.m_mutex);
			m.m_enabledLevels[static_cast<size_t>(level)] = enabled;
		}
	}


}