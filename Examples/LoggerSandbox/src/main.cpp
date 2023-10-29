#include "Logger.h"
#include <iostream>

int main(void)
{
	Log::ContextLogger logger("TestLogger");

	Log::ContextLogger* id1 = logger.createContext("TestContext");
	id1->log("Test");
	id1->log("Warning msg", Log::Level::warning);
	id1->tabIn();
	id1->log("Info msg", Log::Level::info);
	id1->tabIn();
	_sleep(100);

	Log::ContextLogger* id2 = logger.createContext("TestContext 2");
	id2->log("Error msg1", Log::Level::error);
	id2->log("Error msg2", Log::Level::error);
	id2->log("Warning msg", Log::Level::warning);
	id2->tabIn();
	id2->log("Info msg", Log::Level::info);
	id2->tabIn();
	_sleep(1000);

	Log::ContextLogger* id3 = id2->createContext("TestContext 3");
	id3->log("Error msg1", Log::Level::error);
	id3->log("Error msg2", Log::Level::error);
	id3->log("Warning msg", Log::Level::warning);
	id3->tabIn();
	id3->log("Info msg", Log::Level::info);
	id3->tabIn();
	_sleep(100);
	id3->log("Error msg1", Log::Level::error);
	id3->log("Error msg2", Log::Level::error);
	id1->setTabCount(0);
	id1->log("Log context 1");


	std::cout << logger;

	std::cout << "end\n";

	getchar();
	return 0;
}