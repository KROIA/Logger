#include "AbstractReceiver.h"
#include "LogManager.h"
#include "Utilities/MessageFilter.h"

namespace Log
{
	namespace Internal
	{
		SignalReceiver::SignalReceiver(AbstractReceiver* receiver)
			: receiver(receiver)
			, m_levelFilter()
		{
			LogManager& m = LogManager::instance();
			connect(&m, &LogManager::onNewLogger, this, &SignalReceiver::onNewLogger, Qt::QueuedConnection);
			connect(&m, &LogManager::onLoggerInfoChanged, this, &SignalReceiver::onLoggerInfoChanged, Qt::QueuedConnection);
			connect(&m, &LogManager::onLogMessage, this, &SignalReceiver::onLogMessage, Qt::QueuedConnection);
			connect(&m, &LogManager::onChangeParent, this, &SignalReceiver::onChangeParent, Qt::QueuedConnection);

			for (size_t i = 0; i < m_levelFilter.size(); i++)
				m_levelFilter[i] = true;
		}

		void SignalReceiver::setLevelFilter(Level level, bool enable)
		{
			m_levelFilter[(size_t)level] = enable;
		}

		void SignalReceiver::onNewLogger(LogObject::Info loggerInfo) { receiver->onNewLogger(loggerInfo); }
		void SignalReceiver::onLoggerInfoChanged(LogObject::Info info) { receiver->onLoggerInfoChanged(info); }
		void SignalReceiver::onLogMessage(Message message) 
		{ 
			Level level = message.getLevel();
			if(level >= Level::__count)
				return;
			if (!m_levelFilter[(size_t)level])
				return;
			if (m_messageFilter)
			{
				if (!m_messageFilter->filter(message))
					return;
			}
			receiver->onLogMessage(message); 
		}
		void SignalReceiver::onChangeParent(LoggerID childID, LoggerID newParentID) { receiver->onChangeParent(childID, newParentID); }
	}



	AbstractReceiver::AbstractReceiver()
		: signalReceiver(this)
	{
		
	}
	AbstractReceiver::~AbstractReceiver()
	{

	}
	void AbstractReceiver::setLevelFilter(Level level, bool enable)
	{
		signalReceiver.setLevelFilter(level, enable);
	}

	void AbstractReceiver::clearFilter()
	{
		signalReceiver.m_messageFilter = nullptr;
	}

	
}