#include "Logger.h"
#include <iostream>

#include<QMainWindow>
#include<QMenuBar>
#include<QAction>
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

    Log::UI::QConsoleView* view = new Log::UI::QConsoleView();
    mainWindow.setCentralWidget(view);

    // --- Menu bar: Datei -> Öffne Log Datei ---
    QMenuBar* menuBar = mainWindow.menuBar();
    QMenu* dateiMenu = menuBar->addMenu(QObject::tr("Datei"));

    QAction* openAction = new QAction(QString::fromUtf16(u"Öffne Log Datei"), &mainWindow);
    openAction->setShortcut(QKeySequence::Open);        // Ctrl+O
    dateiMenu->addAction(openAction);

    // Connect action: open file dialog, then load the selected file
    QObject::connect(openAction, &QAction::triggered, [&]() {
        const QString filePath = QFileDialog::getOpenFileName(
            &mainWindow,
            QString::fromUtf16(u"Log Datei öffnen"),
            QString(),                                  // start directory (last used / home)
            QObject::tr("Log Dateien (*.log *.txt *.prof);;Alle Dateien (*)")
        );

        if (!filePath.isEmpty()) {
            view->loadMessagesFromFile(filePath.toStdString());
        }
        });

    mainWindow.resize(1024, 768);
    mainWindow.show();

    app.exec();
    Log::Profiler::stop("LogViewer.prof");
    getchar();
    return 0;
}