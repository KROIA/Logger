#include "LogManager.h"
#include <QCoreApplication>
#include <QThread>

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
		LOGGER_SENDER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
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
		LOGGER_SENDER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
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
			if (m.m_enableAutomaticEventProcessing)
			{
				std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
				if (now - m.m_lastEventProcessingTime >= m.m_minTimeBetweenEventProcessing)
				{
					m.m_lastEventProcessingTime = now;
					processEventsIfNoEventLoopRunning();
				}
			}
		}
	}
	void LogManager::onChangeParentInternal(LoggerID childID, LoggerID newParentID)
	{
		LOGGER_SENDER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
		LogManager& m = instance();
		bool emitSignal = false;
		{
			const std::lock_guard<std::mutex> lock(m.m_mutex);
			const auto& it = m.m_logObjects.find(childID);
			// A cyclic parent chain is not survivable downstream: the tree view
			// would reparent a QTreeWidgetItem under its own descendant, and
			// every ancestor walk here would run forever (ISS-006). Keep the
			// previous parent and emit nothing rather than corrupt the tree.
			if (it != m.m_logObjects.end() && !m.wouldCreateParentCycle(childID, newParentID))
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

	bool LogManager::wouldCreateParentCycle(LoggerID childID, LoggerID newParentID) const
	{
		if (newParentID == 0)
			return false;
		// Walk up from the prospective parent. Hitting the child means the
		// child would become its own ancestor.
		LoggerID current = newParentID;
		for (size_t step = 0; step <= m_logObjects.size(); ++step)
		{
			if (current == childID)
				return true;
			if (current == 0)
				return false;
			const auto& it = m_logObjects.find(current);
			if (it == m_logObjects.end())
				return false;
			current = it->second.parentId;
		}
		// A chain longer than the number of known loggers is already cyclic.
		return true;
	}

	void LogManager::setLogObjectInfo(LogObject::Info info)
	{
		LOGGER_SENDER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
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

	bool LogManager::isChildOf(LoggerID childID, LoggerID parentID)
	{
		LogManager& m = instance();
		std::lock_guard<std::mutex> lock(m.m_mutex);
		auto it = m.m_logObjects.find(childID);
		// Bounded by the number of loggers: a longer walk means the parent
		// chain is cyclic, and an unbounded loop would hang here (ISS-006).
		for (size_t step = 0; it != m.m_logObjects.end() && step <= m.m_logObjects.size(); ++step)
		{
			if (it->second.parentId == parentID)
				return true;
			it = m.m_logObjects.find(it->second.parentId);
		}
		return false;
	}
	bool LogManager::processEventsIfNoEventLoopRunning(QEventLoop::ProcessEventsFlags flags, int maxTimeMs)
	{
		// No QApplication/QCoreApplication exists
		if (!QCoreApplication::instance())
			return false;

		QThread* thread = QThread::currentThread();

		// loopLevel() > 0 means an event loop is currently running in this thread
		if (thread->loopLevel() > 0)
			return false;

		// No event loop is running, so process pending events manually
		if (maxTimeMs > 0)
			QCoreApplication::processEvents(flags, maxTimeMs);
		else
			QCoreApplication::processEvents(flags);

		return true;
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