#include <QApplication>

#include "Logger.h"


int main(int argc, char *argv[])
{
	// Qt Compatibility for high DPI displays
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	QApplication app(argc, argv);

	// Create a console view
	Log::UI::NativeConsoleView plotter;

	Log::UI::QTreeConsoleView treeViewConsole;
	treeViewConsole.show();

	// Create a logger object
	Log::LogObject logger("logger OBJ");
	logger.setColor(Log::Colors::Console::Foreground::cyan);
	
	logger.log("This is a simple info message");
	logger.logInfo("This is also a simple info message");
	logger.logWarning("This is a warning message");
	logger.logError("This is an error message");
	logger.logDebug("This is a debug message");
	logger.logCustom("This is a custom message");
	logger.logTrace("This is a trace message");

	logger.log("This is a error message", Log::Level::error);
	logger.log("This is a warning message", Log::Level::warning, Log::Colors::Console::Background::green);	

	// Qt event loop is needed since the logger has emited message signals to the console
	return app.exec();
}