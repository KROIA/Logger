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
    QFile styleFile("E:\\Dokumente\\Visual Studio 2022\\Projects\\Logger\\bin\\styles\\darkstyle.qss"); // Load stylesheet from resources
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&styleFile);
        qApp->setStyleSheet(stream.readAll());
        styleFile.close();
    }
    else {
        QFile styleFile("D:\\Users\\Alex\\Dokumente\\SoftwareProjects\\Logger\\bin\\styles\\darkstyle.qss"); // Load stylesheet from resources
        if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&styleFile);
            qApp->setStyleSheet(stream.readAll());
            styleFile.close();
        }
        else {
            qDebug() << "Failed to load stylesheet."; 
        }
    }
    Log::Color::setDarkMode(true);

    int count = 0;
    bool swtch = false;

    /*QTimer* timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, [&]() {
        // Set up a dark color palette

        QPalette darkPalette = qApp->palette();

        if (swtch)
			darkPalette.setColor((QPalette::ColorRole)count, QColor(255, 25, 25));
		else
            darkPalette.setColor((QPalette::ColorRole)count, QColor(53, 53, 255));
        count++;
        if (count >= QPalette::ColorRole::NColorRoles)
        {
            swtch = !swtch;
            count = 0;
        }
        // Apply the dark color palette to the application
        qApp->setPalette(darkPalette);
	});
    timer->start(100);*/
    

	Log::Logger::ContextLogger logger1("TestLogger1");
	Log::Logger::ContextLogger logger2("TestLogger2");
	logger1.setColor(Log::Color::orange);
	logger2.setColor(Log::Color::cyan);
	Log::UI::QContextLoggerTreeView* view = new Log::UI::QContextLoggerTreeView();
	view->show();
	Context1Object *obj1 = new Context1Object(logger1,view);
	Context2Object *obj2 = new Context2Object(logger2,view);

	view->attachLogger(logger1);
	view->attachLogger(logger2);

    //Log::UI::QConsoleView* console = new Log::UI::QConsoleView();
    //console->attachLogger(logger1);
    //console->attachLogger(logger2);
    //console->show();
	

	app.exec();
	getchar();
	return 0;
}

/*
#include <QtWidgets>
#include <QTimer>

struct LogEntry {
    QDateTime timestamp;
    QString category;
    QString message;
};

class LogModel : public QAbstractItemModel {
public:
    explicit LogModel(QObject* parent = nullptr) : QAbstractItemModel(parent) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;
        return logs.size();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override {
        return 3; // Timestamp, Category, Message
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= logs.size() || index.column() >= 3)
            return QVariant();

        const LogEntry* entry = logs[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) 
            {
                case 0: return entry->timestamp.toString("yyyy-MM-dd hh:mm:ss");
                case 1: return entry->category;
                case 2: return entry->message;
            }
            break;
        case Qt::ForegroundRole:
            switch (index.column()) 
            {
                case 0: 
                    return QBrush(Qt::red);
                case 1: 
                    return QBrush(Qt::green);
                case 2: 
                    return QBrush(Qt::blue);
            }
            break;
        default:
            return QVariant();
        }
        return QVariant();
    }

    void addLog(const LogEntry& entry) {
        beginInsertRows(QModelIndex(), logs.size(), logs.size());
        logs.push_back(new LogEntry(entry));
        endInsertRows();
    }

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override {
		if (parent.isValid() || row < 0 || row >= logs.size() || column < 0 || column >= 3)
			return QModelIndex();
		return createIndex(row, column);
	}

    QModelIndex parent(const QModelIndex& child) const override {
		Q_UNUSED(child);
		return QModelIndex();
	}

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            switch (section) {
            case 0: return "Timestamp";
            case 1: return "Category";
            case 2: return "Message";
            default: break;
            }
        }
        return QVariant();
    }


private:
    std::vector<LogEntry*> logs;
};


class CustomDelegate : public QStyledItemDelegate {
public:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyledItemDelegate::paint(painter, option, index);
        painter->setPen(option.palette.color(QPalette::Text)); // Reset pen color
    }
};

class CustomConsole : public QTableView {
public:

    CustomConsole(QWidget* parent = nullptr) : QTableView(parent) {
        // Set up model
        model = new LogModel(this);
        setModel(model);
        //this->verticalHeader()->setVisible(false);
        this->setShowGrid(false);
        this->verticalHeader()->setDefaultSectionSize(15);
        //this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ///setItemDelegate(new CustomDelegate); // Set custom delegate
        //setWordWrap(true); // Enable word wrapping for long messages
        
        connect(&timer, &QTimer::timeout, this, &CustomConsole::onTimer);
        timer.start(100);
    }

    void addLog(const LogEntry& entry) {
        model->addLog(entry);
        //scrollToBottom();
        changes = true;
    }

private slots:
    void onTimer()
    {
        if (changes)
        {
            scrollToBottom();
            changes = false;
        }
    }

private:
    QTimer timer;
    bool changes = false;
    LogModel* model;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    CustomConsole tableView;
    tableView.show();

    // Example usage:
    for (int i = 0; i < 1000; ++i) {
        LogEntry entry{ QDateTime::currentDateTime(), "Info", QString("Log message %1").arg(i) };
        tableView.addLog(entry);
    }
    tableView.resizeColumnsToContents();

    return app.exec();
}*/