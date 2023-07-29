#include "Logger.h"

int main(void)
{
	Log::ConsoleLogger AbstractLogger;

	AbstractLogger.log("Test");
	AbstractLogger.log("Warning msg", Log::Level::warning);
	AbstractLogger.tabIn();
	AbstractLogger.log("Info msg", Log::Level::info);
	AbstractLogger.tabIn();
	_sleep(100);
	AbstractLogger.log("Error msg1", Log::Level::error);
	AbstractLogger.log("Error msg2", Log::Level::error);
	AbstractLogger.log("Warning msg", Log::Level::warning);
	AbstractLogger.tabIn();
	AbstractLogger.log("Info msg", Log::Level::info);
	AbstractLogger.tabIn();
	_sleep(1000);
	AbstractLogger.log("Error msg1", Log::Level::error);
	AbstractLogger.log("Error msg2", Log::Level::error);
	AbstractLogger.log("Warning msg", Log::Level::warning);
	AbstractLogger.tabIn();
	AbstractLogger.log("Info msg", Log::Level::info);
	AbstractLogger.tabIn();
	_sleep(100);
	AbstractLogger.log("Error msg1", Log::Level::error);
	AbstractLogger.log("Error msg2", Log::Level::error);

	getchar();
	return 0;
}