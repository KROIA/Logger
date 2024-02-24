#pragma once
#include "Logger_base.h"
#include "ContextReceiver.h"

namespace Log
{
	namespace Receiver
	{
		class LOGGER_EXPORT ConsoleContextPlotter : public ContextReceiver
		{
		public:
			ConsoleContextPlotter();
			ConsoleContextPlotter(const ConsoleContextPlotter& other);

			~ConsoleContextPlotter();

		private:
			void onContextCreate(Logger::ContextLogger& logger) override;
			void onContextDestroy(Logger::ContextLogger& logger) override;


			void onNewSubscribed(Logger::AbstractLogger& logger) override;
			void onUnsubscribed(Logger::AbstractLogger& logger) override;

			void onNewMessage(const Message& m) override;
			void onClear(Logger::AbstractLogger& logger) override;

			void onDelete(Logger::AbstractLogger& logger) override;

			void printToConsole(const Message& msg);
		};
	}
}