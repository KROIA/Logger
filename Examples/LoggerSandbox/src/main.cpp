#include "Logger.h"
#include "Context1Object.h"
#include "Context2Object.h"
#include <iostream>

#include<QTreeWidget>
#include<QApplication>
#include <QIcon>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QThread>

#include "ui/DateTimeWidget.h"

int main(int argc, char* argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);


	QApplication app(argc, argv);

    // Set up a dark color palette
   /* QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(90, 90, 90));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    for (int i = 0; i < QPalette::NColorRoles; i++)
    {
		darkPalette.setColor((QPalette::ColorRole)i, QColor(255, 25, 25));
	}

    // Apply the dark color palette to the application
    qApp->setPalette(darkPalette);*/

    // Apply a dark stylesheet for the application
  
    qApp->setStyleSheet(Log::Resources::getDarkStylesheet());
    Log::Color::setDarkMode(true);

    int count = 0;
    bool swtch = false;
    

	Log::Logger::ContextLogger logger1("TestLogger1");
	Log::Logger::ContextLogger logger2("TestLogger2");
	Log::Logger::AbstractLogger logger3("AbstractLogger");
    logger1.setIcon(QIcon(":\\icons\\debug.png"));
    logger2.setIcon(QIcon(":\\icons\\trace.png"));
	logger1.setColor(Log::Color::orange);
	logger2.setColor(Log::Color::cyan);
	Log::UI::QContextLoggerTreeView* view = new Log::UI::QContextLoggerTreeView();
	view->show();

    Log::UI::QConsoleView* console = new Log::UI::QConsoleView();
    console->attachLogger(logger1);
    console->attachLogger(logger2);
    console->attachLogger(logger3);
    console->show();

	Context1Object *obj1 = new Context1Object(logger1,view, console);
	Context2Object *obj2 = new Context2Object(logger2,view, console);

    logger3.log(Log::Level::info, Log::Color::green, "Hallo");

	view->attachLogger(logger1);
	view->attachLogger(logger2);
	view->attachLogger(logger3);

    QTimer singleShotTimer;
    singleShotTimer.setSingleShot(true);
    QObject::connect(&singleShotTimer, &QTimer::timeout, [&]()
    {
        // Create a QThread that calls the heavy commands
        QThread* m_workerThread = new QThread;
        // worker lambda
        auto worker = [m_workerThread, &logger2]()
            {
                
                Log::Logger::ContextLogger* context = logger2.createContext("Utilities::executeCommand");
                

                for(int i = 0; i < 2; i++)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    context->log(Log::Level::info, Log::Color::green, "Hallo\n\n\n");
                }
                

              
                context->deleteLater();
                
                m_workerThread->exit();
            };
        // move worker to thread
        QObject::connect(m_workerThread, &QThread::started, worker);
        // delete after thread is finished
        QObject::connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
        m_workerThread->start();
    });
    singleShotTimer.start(100);
    
	
	app.exec();
	getchar();
	return 0;
}