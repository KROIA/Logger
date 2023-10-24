#include "Logger.h"
#include <iostream>

int main(void)
{
	Log::ContextLogger logger("TestLogger");

	Log::ContextLogger::ContextID id1 = logger.createContext("TestContext");
	logger.switchContext(id1);
	logger.log("Test");
	logger.log("Warning msg", Log::Level::warning);
	logger.tabIn();
	logger.log("Info msg", Log::Level::info);
	logger.tabIn();
	_sleep(100);

	Log::ContextLogger::ContextID id2 = logger.createContext("TestContext 2");
	logger.switchContext(id2);
	logger.log("Error msg1", Log::Level::error);
	logger.log("Error msg2", Log::Level::error);
	logger.log("Warning msg", Log::Level::warning);
	logger.tabIn();
	logger.log("Info msg", Log::Level::info);
	logger.tabIn();
	_sleep(1000);

	Log::ContextLogger::ContextID id3 = logger.createContext("TestContext 3");
	logger.switchContext(id3);
	logger.log("Error msg1", Log::Level::error);
	logger.log("Error msg2", Log::Level::error);
	logger.log("Warning msg", Log::Level::warning);
	logger.tabIn();
	logger.log("Info msg", Log::Level::info);
	logger.tabIn();
	_sleep(100);
	logger.log("Error msg1", Log::Level::error);
	logger.log("Error msg2", Log::Level::error);
	logger.switchContext(id1);
	logger.setTabCount(0);
	logger.log("Log context 1");


	std::cout << logger;

	std::cout << "end\n";

	getchar();
	return 0;
}