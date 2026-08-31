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

int main(int argc, char* argv[])
{
    Log::Profiler::start();
    // High-DPI scaling and pixmaps are always on in Qt 6; the attributes are deprecated no-ops there.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
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
    Log::FilePlotter filePlotter("log.txt");

    int count = 0;
    bool swtch = false;
    

	Log::LogObject logger1("TestLogger1");
	Log::LogObject logger2("TestLogger2");
	Log::LogObject logger3("AbstractLogger");
    //logger1.setIcon(QIcon(":\\icons\\debug.png"));
    //logger2.setIcon(QIcon(":\\icons\\trace.png"));
	logger1.setColor(Log::Colors::orange);
	logger2.setColor(Log::Colors::cyan);
    logger3.setColor(Log::Colors::brown);
	Log::UI::QTreeConsoleView* view = new Log::UI::QTreeConsoleView();
	view->show();


    Log::UI::QConsoleView* console = new Log::UI::QConsoleView();
    //console->attachLogger(logger1);
    //console->attachLogger(logger2);
    //console->attachLogger(logger3);
    console->show();

    Log::UI::QCombinedConsoleView* combined = new Log::UI::QCombinedConsoleView();
    combined->show();

    Log::UI::QStatsConsoleView* stats = new Log::UI::QStatsConsoleView();
    stats->show();

    logger3.logError("Hello, this is a long message that should be wrapped in the console view\nmultiple lines shuld also be possible");

	Context1Object *obj1 = new Context1Object(logger1,view, console);
	Context2Object *obj2 = new Context2Object(logger2,view, console);

    logger3.log("Hallo", Log::Level::info, Log::Colors::green);

	//view->attachLogger(logger1);
	//view->attachLogger(logger2);
	//view->attachLogger(logger3);

    // --- ReceiverVisibilityPolicy / ContextDisplayPolicy sandbox ---
    // A ManualAdd child: every receiver still tracks it, but keeps it hidden
    // until revealed (tree/table context menu "Show hidden logger", or the
    // sidebar checkbox).
    Log::LogObject manualAddLogger(logger1.getID(), "ManualAddChild");
    manualAddLogger.setColor(Log::Colors::lightBlue);
    manualAddLogger.setVisibilityPolicy(Log::ReceiverVisibilityPolicy::ManualAdd);

    // An Invisible grandchild nested under it: no receiver ever shows it.
    Log::LogObject invisibleLogger(manualAddLogger.getID(), "InvisibleGrandchild");
    invisibleLogger.setColor(Log::Colors::red);
    invisibleLogger.setVisibilityPolicy(Log::ReceiverVisibilityPolicy::Invisible);

    // ...with its own AutoVisible child, to exercise reparenting past an
    // invisible ancestor: this logger's row should attach to the nearest
    // visible ancestor (manualAddLogger), not vanish along with invisibleLogger.
    Log::LogObject visibleUnderInvisible(invisibleLogger.getID(), "VisibleUnderInvisible");
    visibleUnderInvisible.setColor(Log::Colors::yellow);

    // A FlattenSuggested logger: hierarchical views may fold its messages
    // into its nearest visible ancestor's context instead of giving it its
    // own row.
    Log::LogObject flattenLogger(logger3.getID(), "FlattenSuggestedLogger");
    flattenLogger.setColor(Log::Colors::magenta);
    flattenLogger.setContextDisplayPolicy(Log::ContextDisplayPolicy::FlattenSuggested);

    manualAddLogger.logInfo("Message from ManualAdd logger (hidden by default)");
    invisibleLogger.logInfo("Message from Invisible logger (never shown by any receiver)");
    visibleUnderInvisible.logInfo("Message from a visible child of an Invisible logger");
    flattenLogger.logInfo("Message from FlattenSuggested logger (folded into its parent)");
    flattenLogger.logWarning("Second FlattenSuggested message, still folded");

    QTimer singleShotTimer;
    singleShotTimer.setSingleShot(true);
    QObject::connect(&singleShotTimer, &QTimer::timeout, [&]()
    {
        // Create a QThread that calls the heavy commands
        QThread* m_workerThread = new QThread;
        // worker lambda
        auto worker = [m_workerThread, &logger2]()
            {
                
                Log::LogObject* context = new Log::LogObject(logger2, "Utilities::executeCommand");
                

                for(int i = 0; i < 2; i++)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    context->log("Hallo\n\n\n", Log::Level::info, Log::Colors::green);
                }
                

              
                //context->deleteLater();
                
                m_workerThread->exit();
            };
        // move worker to thread
        QObject::connect(m_workerThread, &QThread::started, worker);
        // delete after thread is finished
        QObject::connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
        m_workerThread->start();
    });
    singleShotTimer.start(100);

    // Flip a policy at runtime, after messages already exist under it, to
    // exercise the reconciliation/migration path (not just creation-time
    // handling): the flattened logger regains its own tree context and its
    // already-emitted messages migrate back out of its parent's context.
    QTimer flattenFlipTimer;
    flattenFlipTimer.setSingleShot(true);
    QObject::connect(&flattenFlipTimer, &QTimer::timeout, [&]()
    {
        flattenLogger.setContextDisplayPolicy(Log::ContextDisplayPolicy::OwnContext);
        flattenLogger.logInfo("Materialized under its own context (policy flipped at runtime)");
    });
    flattenFlipTimer.start(3000);


	app.exec();
    Log::Profiler::stop("Loggersandbox.prof");
	getchar();
	return 0;
}