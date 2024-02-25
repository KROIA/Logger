#include "Logger.h"
#include "Context1Object.h"
#include "Context2Object.h"
#include <iostream>

#include<QTreeWidget>
#include<QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

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

	

	app.exec();
	getchar();
	return 0;
}