#include "Logger.h"
#include <iostream>

#include<QTreeWidget>
#include<QApplication>
#include <QIcon>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QThread>

int main(int argc, char* argv[])
{
    Log::Profiler::start();
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);


	QApplication app(argc, argv);

    // Apply a dark stylesheet for the application
  
    qApp->setStyleSheet(Log::Resources::getDarkStylesheet());
    Log::Color::setDarkMode(true);
   // Log::FilePlotter filePlotter("log.txt");

    int count = 0;
    bool swtch = false;
    

	Log::LogObject logger("TestLogger1");
	logger.setColor(Log::Colors::orange);
	Log::UI::QTreeConsoleView* view = new Log::UI::QTreeConsoleView();
	view->show();

    Log::UI::QConsoleView* console = new Log::UI::QConsoleView();
    console->show();

    Log::LogManager::setEnableAutomaticEventProcessing(false);
    for (unsigned int i = 0; i < 32000; i++)
    {
        Log::Message message("This is a test message " + std::to_string(count++), Log::Level::info);
        logger.log(message);
    }

    QTimer  timer;
    QObject::connect(&timer, &QTimer::timeout, [&]()
        {
            Log::Message message("This is a test message " + std::to_string(count++), Log::Level::info);
            logger.log(message);
            
		});
	timer.start(10); // 0 ms interval for maximum speed
    
    
	
	app.exec();
    Log::Profiler::stop("PerformanceTest.prof");
	getchar();
	return 0;
}