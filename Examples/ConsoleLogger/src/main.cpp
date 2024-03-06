#include "Logger.h"

int main(void)
{
	Log::Logger::ContextLogger context("context");
	
	Log::Logger::AbstractLogger context3("context3");
	context.setColor(Log::Color::Console::Foreground::cyan);
	//Log::Receiver::ConsolePlotter plotter;
	Log::Receiver::ConsoleContextPlotter plotter;
	plotter.attachLogger(context);
	plotter.attachLogger(context3);

	context3.log("Test123");
	context.log("Test");
	_sleep(10);
	context.log(Log::Level::warning, "Warning msg");
	_sleep(10);
	//context.tabIn();
	context.log(Log::Level::info, "Info msg");
	context.log(Log::Level::info, Log::Color::green, "Info msg green");
	//context.tabIn();
	_sleep(100);
	Log::Logger::ContextLogger *context2 = context.createContext("name2");
	context2->setColor(Log::Color::Console::Background::green);
	context2->log(Log::Level::error, "Error msg1");
	_sleep(10);
	context2->log(Log::Level::error, "Error msg2");
	_sleep(10);
	context2->log(Log::Level::warning, "Warning msg");
	_sleep(10);
	//context2->tabIn();
	//context2->tabIn();
	context2->log(Log::Level::info, "Info msg");
	//context2->tabIn();
	_sleep(100);
	context.log(Log::Level::error, "Error msg3");
	_sleep(10);
	context.log(Log::Level::error, "Error msg4");
	_sleep(10);
	context.log(Log::Level::warning, "Warning msg" );
	_sleep(10);
	context.tabIn();
	context.log(Log::Level::info, "Info msg");
	_sleep(10);
	context.tabIn();
	_sleep(100);
	context.log(Log::Level::error, "Error msg5");
	_sleep(10);
	context.log(Log::Level::error, "Error msg6");
	_sleep(100);
	context2->log(Log::Level::info, "Info msg");
	_sleep(10);

	

	getchar();
	return 0;
}