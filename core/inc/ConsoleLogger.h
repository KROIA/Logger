#pragma once

#include "Logger.h"

namespace Log
{
	class LOGGER_EXPORT ConsoleLogger : public Logger
	{
	public:
		ConsoleLogger();
		ConsoleLogger(const ConsoleLogger& other);

		~ConsoleLogger();

	private:
		void logInternal(const Message& msg) override;
	};
}