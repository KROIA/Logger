#include <QApplication>

#include "Logger.h"
#include <QTimer>


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// Create a console view
	Log::UI::NativeConsoleView plotter;
	Log::UI::QTreeConsoleView treeConsole;
	treeConsole.show();

	// Create 2 logger objects
	Log::LogObject logger1("logger OBJ 1");
	Log::LogObject logger2("logger OBJ 2");
	

	logger1.setColor(Log::Colors::Console::Foreground::cyan);
	logger2.setColor(Log::Colors::Console::Foreground::magenta);

	
	logger1.log("This is a simple info message");
	logger1.logInfo("This is also a simple info message");
	logger1.logWarning("This is a warning message");
	logger1.logError("This is an error message");
	logger1.logDebug("This is a debug message");
	logger1.logCustom("This is a custom message");
	logger1.logTrace("This is a trace message");

	logger1.log("This is a error message", Log::Level::error);
	logger1.log("This is a warning message", Log::Level::warning, Log::Colors::Console::Background::green);	

	logger2.log("This is a simple info message");
	logger1.log("Now we will change the parent of logger2 to logger1");

	logger2.setParentID(logger1.getID());

	logger2.log("This logger is now a child of logger 1");


	// Qt event loop is needed since the logger has emited message signals to the console
	return app.exec();
}