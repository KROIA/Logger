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

			// Snapshot loggers that were registered before this receiver subscribed.
			// Queue the forwarding through the event loop so it runs after the
			// derived AbstractReceiver is fully constructed (avoids pure-virtual dispatch).
			// Handlers on the receiver side must tolerate duplicates for the small
			// window between connect() and the snapshot.
			// Route through the gated slots (not receiver directly) so the
			// filter applies. QueuedConnection defers these past the synchronous
			// setFilter(...) call, so the filter is installed when they run.
			SignalReceiver* self = this;
			std::vector<LogObject::Info> existing = LogManager::getLogObjectsInfo();
			for (const LogObject::Info& info : existing)
			{
				LogObject::Info infoCopy = info;
				QMetaObject::invokeMethod(this, [self, infoCopy]() {
					self->onNewLogger(infoCopy);
				}, Qt::QueuedConnection);
				if (info.parentId != 0)
				{
					LoggerID cid = info.id;
					LoggerID pid = info.parentId;
					QMetaObject::invokeMethod(this, [self, cid, pid]() {
						self->onChangeParent(cid, pid);
					}, Qt::QueuedConnection);
				}
			}
		}

		void SignalReceiver::setLevelFilter(Level level, bool enable)
		{
			m_levelFilter[(size_t)level] = enable;
		}

		void SignalReceiver::onNewLogger(LogObject::Info loggerInfo)
		{
			if (m_messageFilter && !m_messageFilter->filterByLoggerID(loggerInfo.id))
				return;
			receiver->onNewLogger(loggerInfo);
		}
		void SignalReceiver::onLoggerInfoChanged(LogObject::Info info)
		{
			if (m_messageFilter && !m_messageFilter->filterByLoggerID(info.id))
				return;
			receiver->onLoggerInfoChanged(info);
		}
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
		void SignalReceiver::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			if (m_messageFilter && !m_messageFilter->filterByLoggerID(childID))
				return;
			receiver->onChangeParent(childID, newParentID);
		}
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