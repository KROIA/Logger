#include "ConsoleLogger.h"

int main(void)
{
	Log::ConsoleLogger logger;

	logger.log("Test");
	logger.log("Warning msg", Log::Level::warning);
	logger.tabIn();
	logger.log("Info msg", Log::Level::info);
	logger.tabIn();
	_sleep(100);
	logger.log("Error msg1", Log::Level::error);
	logger.log("Error msg2", Log::Level::error);
	logger.log("Warning msg", Log::Level::warning);
	logger.tabIn();
	logger.log("Info msg", Log::Level::info);
	logger.tabIn();
	_sleep(1000);
	logger.log("Error msg1", Log::Level::error);
	logger.log("Error msg2", Log::Level::error);
	logger.log("Warning msg", Log::Level::warning);
	logger.tabIn();
	logger.log("Info msg", Log::Level::info);
	logger.tabIn();
	_sleep(100);
	logger.log("Error msg1", Log::Level::error);
	logger.log("Error msg2", Log::Level::error);

	getchar();
	return 0;
}