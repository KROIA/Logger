#include <QCoreapplication>

#include "Logger.h"


int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	Log::LogObject context("context");
	
	Log::LogObject context3("context3");
	context.setColor(Log::Colors::Console::Foreground::cyan);
	//Log::Receiver::ConsolePlotter plotter;
	Log::UI::NativeConsoleView plotter;


	context3.log("Test123");
	context.log("Test");
	_sleep(10);
	context.log("Warning msg", Log::Level::warning);
	_sleep(10);
	//context.tabIn();
	context.log("Info msg", Log::Level::info);
	context.log("Info msg green", Log::Level::info, Log::Colors::green);
	//context.tabIn();
	_sleep(100);
	Log::LogObject *context2 = new Log::LogObject(context, "Name 2");
	context2->setColor(Log::Colors::Console::Background::green);
	context2->log("Error msg1", Log::Level::error);
	_sleep(10);
	context2->log("Error msg2", Log::Level::error);
	_sleep(10);
	context2->log("Warning msg", Log::Level::warning);
	_sleep(10);
	//context2->tabIn();
	//context2->tabIn();
	context2->log("Info msg", Log::Level::info);
	//context2->tabIn();
	_sleep(100);
	context.log("Error msg3", Log::Level::error);
	_sleep(10);
	context.log("Error msg4", Log::Level::error);
	_sleep(10);
	context.log("Warning msg", Log::Level::warning);
	_sleep(10);
	//context.tabIn();
	context.log("Info msg", Log::Level::info);
	_sleep(10);
	//context.tabIn();
	_sleep(100);
	context.log("Error msg5", Log::Level::error);
	_sleep(10);
	context.log("Error msg6", Log::Level::error);
	_sleep(100);
	context2->log("Info msg", Log::Level::info);
	_sleep(10);

	

	return app.exec();
}