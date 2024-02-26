#pragma once
#include "Logger_base.h"
#include "LoggerTypes/AbstractLogger.h"
#include <vector>

namespace Log
{
	namespace Receiver
	{
		class LOGGER_EXPORT AbstractReceiver
		{
		public:
			virtual ~AbstractReceiver();

			virtual void attachLogger(Logger::AbstractLogger& logger);
			virtual void detachLogger(Logger::AbstractLogger& logger);

			

			virtual void clearAttachedLoggers();
			const std::vector<Logger::AbstractLogger*>& getAttachedLoggers() const;

			bool isAttached(Logger::AbstractLogger& logger) const;
		protected:
			virtual void onPrintAllMessages(Logger::AbstractLogger& logger);

			virtual void onNewSubscribed(Logger::AbstractLogger& logger) = 0;
			virtual void onUnsubscribed(Logger::AbstractLogger& logger) = 0;

			virtual void onNewMessage(const Message& m) = 0;
			virtual void onClear(Logger::AbstractLogger& logger) = 0;
			virtual void onDelete(Logger::AbstractLogger& logger) = 0;


			std::vector<Logger::AbstractLogger*> m_loggers;
		private:
			void onDeletePrivate(Logger::AbstractLogger& logger);
			

		};
	}
}