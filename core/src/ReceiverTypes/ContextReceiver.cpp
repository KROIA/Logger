#include "ReceiverTypes/ContextReceiver.h"


namespace Log
{
	namespace Receiver
	{
		ContextReceiver::~ContextReceiver()
		{
			clearAttachedLoggers();
		}

		void ContextReceiver::attachLogger(Logger::AbstractLogger& logger)
		{
			if (isAttached(logger))
				return;
			Logger::ContextLogger *contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
			if (contextLogger)
			{
				contextLogger->connect_onContextCreate_slot(this, &ContextReceiver::onContextCreate);
				contextLogger->connect_onContextDestroy_slot(this, &ContextReceiver::onContextDestroy);
			}
			AbstractReceiver::attachLogger(logger);
		}
		void ContextReceiver::detachLogger(Logger::AbstractLogger& logger)
		{
			if (!isAttached(logger))
				return;
			Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
			if (contextLogger)
			{
				contextLogger->disconnect_onContextCreate_slot(this, &ContextReceiver::onContextCreate);
				contextLogger->disconnect_onContextDestroy_slot(this, &ContextReceiver::onContextDestroy);
			}
			AbstractReceiver::detachLogger(logger);
		}

		void ContextReceiver::clearAttachedLoggers()
		{
			for (size_t i = 0; i < m_loggers.size(); i++)
			{
				Logger::ContextLogger * contextLogger = dynamic_cast<Logger::ContextLogger*>(m_loggers[i]);
				if (contextLogger)
				{
					contextLogger->disconnect_onContextCreate_slot(this, &ContextReceiver::onContextCreate);
					contextLogger->disconnect_onContextDestroy_slot(this, &ContextReceiver::onContextDestroy);
				}
			}
			AbstractReceiver::clearAttachedLoggers();
		}
		void ContextReceiver::onPrintAllMessagesRecursive(Logger::ContextLogger& logger)
		{
			std::vector<Message> messages;
			logger.getMessagesRecursive(messages);
			Message::sort(messages, Message::SortType::timeAscending);
			for (size_t i = 0; i < messages.size(); ++i)
			{
				onNewMessage(messages[i]);
			}
		}
	}
}