#pragma once
#include "Logger_base.h"
#include "AbstractReceiver.h"

namespace Log
{
	namespace Receiver
	{
		class LOGGER_EXPORT ConsolePlotter : public AbstractReceiver
		{
		public:
			ConsolePlotter();
			ConsolePlotter(const ConsolePlotter& other);

			~ConsolePlotter();

		private:
			void onNewSubscribed(Logger::AbstractLogger& logger) override;
			void onUnsubscribed(Logger::AbstractLogger& logger) override;

			//void onContextCreate(Logger::ContextLogger& newContext) override;
			void onNewMessage(const Message& m) override;
			void onClear(Logger::AbstractLogger& logger) override;
			//void onContextDestroy(Logger::ContextLogger& logger) override;
			void onDelete(Logger::AbstractLogger& logger) override;

			void printToConsole(const Message& msg);
		};
	}
}