#include <QCoreapplication>

#include "Logger.h"


int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	Log::LogObject logger1("logger1");
	logger1.setColor(Log::Colors::Console::Foreground::cyan);

	Log::LogObject logger2("logger2");
	logger2.setColor(Log::Colors::Console::Foreground::magenta);

	Log::UI::NativeConsoleView plotter;

	logger1.log("Hello from logger1");
	logger2.log("Hello from logger2");
	_sleep(10);
	logger1.log("This is a test message");
	logger2.logError("This is an error message");

	

	return app.exec();
}