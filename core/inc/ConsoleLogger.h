#pragma once

#include "AbstractLogger.h"

namespace Log
{
	class LOGGER_EXPORT ConsoleLogger : public AbstractLogger
	{
	public:
		ConsoleLogger();
		ConsoleLogger(const ConsoleLogger& other);

		~ConsoleLogger();

	private:
		void logInternal(const Message& msg) override;
	};
}