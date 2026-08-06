#include "Logger.h"

#include<QMainWindow>
#include<QMenuBar>
#include<QAction>
#include<QActionGroup>
#include<QMenu>
#include<QFileDialog>
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

    // Create main window to host menu bar + central widget
    QMainWindow mainWindow;

    Log::UI::QCombinedConsoleView* view = new Log::UI::QCombinedConsoleView();
    mainWindow.setCentralWidget(view);

    // --- Menu bar: Datei -> Oeffne Log Datei ---
    QMenuBar* menuBar = mainWindow.menuBar();
    QMenu* dateiMenu = menuBar->addMenu(QObject::tr("Datei"));

    QAction* openAction = new QAction(QString::fromUtf16(u"\u00d6ffne Log Datei"), &mainWindow);
    openAction->setShortcut(QKeySequence::Open);        // Ctrl+O
    dateiMenu->addAction(openAction);

    // Connect action: open file dialog, then load the selected file
    QObject::connect(openAction, &QAction::triggered, [&]() {
        const QString filePath = QFileDialog::getOpenFileName(
            &mainWindow,
            QString::fromUtf16(u"Log Datei \u00d6ffnen"),
            QString(),                                  // start directory (last used / home)
            QObject::tr("Log Dateien (*.log *.txt *.prof);;Alle Dateien (*)")
        );

        if (!filePath.isEmpty()) {
            view->loadMessagesFromFile(filePath.toStdString());
        }
        });

    // --- Menu bar: Ansicht -> Present (live) / Past (loaded) ---
    QMenu* ansichtMenu = menuBar->addMenu(QObject::tr("Ansicht"));

    QAction* presentAction = new QAction(QObject::tr("Present (live)"), &mainWindow);
    QAction* pastAction = new QAction(QObject::tr("Past (loaded)"), &mainWindow);
    presentAction->setCheckable(true);
    pastAction->setCheckable(true);
    QActionGroup* modeGroup = new QActionGroup(&mainWindow);
    modeGroup->setExclusive(true);
    modeGroup->addAction(presentAction);
    modeGroup->addAction(pastAction);
    presentAction->setChecked(true);
    ansichtMenu->addAction(presentAction);
    ansichtMenu->addAction(pastAction);

    using TimelineMode = Log::UI::QVerticalTimelineView::Mode;
    QObject::connect(presentAction, &QAction::triggered, [&]() {
        if (auto* tl = view->verticalTimelineView())
            tl->setMode(TimelineMode::Present);
        });
    QObject::connect(pastAction, &QAction::triggered, [&]() {
        if (auto* tl = view->verticalTimelineView())
            tl->setMode(TimelineMode::Past);
        });
    // Reflect the mode the combined view switches to after a file load.
    QObject::connect(openAction, &QAction::triggered, [&]() {
        if (auto* tl = view->verticalTimelineView())
        {
            const bool past = tl->mode() == TimelineMode::Past;
            pastAction->setChecked(past);
            presentAction->setChecked(!past);
        }
        });

    mainWindow.resize(1024, 768);
    mainWindow.show();

    app.exec();
    Log::Profiler::stop("LogViewer.prof");
    return 0;
}