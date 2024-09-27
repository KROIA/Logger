#include <QApplication>

#include "Logger.h"
#include <QTimer>

int main(int argc, char *argv[])
{
	// Qt Compatibility for high DPI displays
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	QApplication app(argc, argv);

	// Create a file plotter
	Log::FilePlotter plotter("outputFile.txt");

	// Create 2 logger objects
	Log::LogObject logger1("logger OBJ 1");
	Log::LogObject logger2(logger1.getID(), "logger OBJ 2");
	
	logger1.setColor(Log::Colors::Console::Foreground::cyan);
	logger2.setColor(Log::Colors::Console::Foreground::magenta);
	
	logger1.log("This is a simple info message");
	logger2.log("Message from child");

	// Qt event loop is needed since the logger has emited message signals to the console
	return app.exec();
}