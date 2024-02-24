#include "ReceiverTypes/AbstractReceiver.h"

namespace Log
{
	namespace Receiver
	{
		AbstractReceiver::~AbstractReceiver()
		{
			clearAttachedLoggers();
		}
		void AbstractReceiver::clearAttachedLoggers()
		{
			std::vector<Logger::AbstractLogger*> loggers = m_loggers;
			for (size_t i = 0; i < loggers.size(); i++)
			{
				loggers[i]->disconnect_onNewMessage_slot(this, &AbstractReceiver::onNewMessage);
				loggers[i]->disconnect_onClear_slot(this, &AbstractReceiver::onClear);
				loggers[i]->disconnect_onDelete_slot(this, &AbstractReceiver::onDelete);
				loggers[i]->disconnect_onDelete_slot(this, &AbstractReceiver::onDeletePrivate);
			}
			m_loggers.clear();
		}
		const std::vector<Logger::AbstractLogger*>& AbstractReceiver::getAttachedLoggers() const
		{
			return m_loggers;
		}
		bool AbstractReceiver::isAttached(Logger::AbstractLogger& logger) const
		{
			for (size_t i = 0; i < m_loggers.size(); i++)
			{
				if (m_loggers[i] == &logger)
				{
					return true;
				}
			}
			return false;
		}
		void AbstractReceiver::attachLogger(Logger::AbstractLogger& logger)
		{
			if(isAttached(logger))
			{
				return;
			}
			m_loggers.push_back(&logger);
			//logger.connect_onContextCreate_slot(this, &AbstractReceiver::onContextCreate);
			logger.connect_onNewMessage_slot(this, &AbstractReceiver::onNewMessage);
			logger.connect_onClear_slot(this, &AbstractReceiver::onClear);
			//logger.connect_onContextDestroy_slot(this, &AbstractReceiver::onContextDestroy);
			//logger.connect_onDelete_slot(this, &AbstractReceiver::onContextDelete);
			logger.connect_onDelete_slot(this, &AbstractReceiver::onDelete);
			logger.connect_onDelete_slot(this, &AbstractReceiver::onDeletePrivate);

			onNewSubscribed(logger);
		}
		void AbstractReceiver::detachLogger(Logger::AbstractLogger& logger)
		{
			//logger.disconnect_onContextCreate_slot(this, &AbstractReceiver::onContextCreate);
			logger.disconnect_onNewMessage_slot(this, &AbstractReceiver::onNewMessage);
			logger.disconnect_onClear_slot(this, &AbstractReceiver::onClear);
			//logger.disconnect_onContextDestroy_slot(this, &AbstractReceiver::onContextDestroy);
			//logger.disconnect_onDelete_slot(this, &AbstractReceiver::onContextDelete);
			logger.disconnect_onDelete_slot(this, &AbstractReceiver::onDelete);
			logger.disconnect_onDelete_slot(this, &AbstractReceiver::onDeletePrivate);

			onUnsubscribed(logger);
			for (size_t i = 0; i < m_loggers.size(); i++)
			{
				if (m_loggers[i] == &logger)
				{
					m_loggers.erase(m_loggers.begin() + i);
					return;
				}
			}
		}

		void AbstractReceiver::onDeletePrivate(Logger::AbstractLogger& logger)
		{
			detachLogger(logger);
		}

		void AbstractReceiver::onPrintAllMessages(Logger::AbstractLogger& logger)
		{
			const std::vector<Message> &messages = logger.getMessages();
			for (size_t i = 0; i < messages.size(); ++i)
			{
				onNewMessage(messages[i]);
			}
		}
	}
}