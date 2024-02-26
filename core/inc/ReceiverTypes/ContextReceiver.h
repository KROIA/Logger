#pragma once
#include "Logger_base.h"
#include "AbstractReceiver.h"
#include "LoggerTypes/ContextLogger.h"

namespace Log
{
	namespace Receiver
	{
		class LOGGER_EXPORT ContextReceiver : public AbstractReceiver
		{
		public:
			~ContextReceiver();

			void attachLogger(Logger::AbstractLogger& logger) override;
			void detachLogger(Logger::AbstractLogger& logger) override;

			virtual void attachLoggerAndChilds(Logger::ContextLogger& logger);
			virtual void detachLoggerAndChilds(Logger::ContextLogger& logger);

			void clearAttachedLoggers() override;
		protected:
			virtual void onPrintAllMessagesRecursive(Logger::ContextLogger& logger);

			virtual void onContextCreate(Logger::ContextLogger& logger) = 0;
			virtual void onContextDestroy(Logger::ContextLogger& logger) = 0;
		private:
		};
	}
}