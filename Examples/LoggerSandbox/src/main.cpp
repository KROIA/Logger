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
	
	/*Log::ContextLogger logger("TestLogger");

	Log::ContextLogger* id1 = logger.createContext("TestContext");
	id1->log("Test");
	id1->log("Warning msg", Log::Level::warning);
	id1->tabIn();
	id1->log("Info msg", Log::Level::info);
	id1->tabIn();
	_sleep(100);

	Log::ContextLogger* id2 = logger.createContext("TestContext 2");
	id2->log("Error msg1", Log::Level::error);
	id2->log("Error msg2", Log::Level::error);
	id2->log("Warning msg", Log::Level::warning);
	id2->tabIn();
	id2->log("Info msg", Log::Level::info);
	id2->tabIn();
	_sleep(1000);

	Log::ContextLogger* id3 = id2->createContext("TestContext 3");
	id3->log("Error msg1", Log::Level::error);
	id3->log("Error msg2", Log::Level::error);
	id3->log("Warning msg", Log::Level::warning);
	id3->tabIn();
	id3->log("Info msg", Log::Level::info);
	id3->tabIn();
	_sleep(100);
	id3->log("Error msg1", Log::Level::error);
	id3->log("Error msg2", Log::Level::error);
	id1->setTabCount(0);
	id1->log("Log context 1");
	*/

	/*QTreeWidget* widget = new QTreeWidget();
	widget->setColumnCount(3);

	for (Log::ContextLogger* l : Log::ContextLogger::getAllLogger())
	{
		Log::QContextLoggerTreeWidgetItem *item = new Log::QContextLoggerTreeWidgetItem(widget);
		item->updateData(*l);
		//widget->addTopLevelItem(l->getTreeWidgetItem());
	}
	widget->show();*/

	//Log::QContextLoggerTreeView * loggerView = new Log::QContextLoggerTreeView();
	//loggerView->updateView();
	//loggerView->show();

	//std::cout << logger;

	//std::cout << "end\n";
	Log::ContextLogger logger1("TestLogger1");
	Log::ContextLogger logger2("TestLogger2");
	Log::QContextLoggerTreeView* view = new Log::QContextLoggerTreeView();
	view->show();
	Context1Object obj1(logger1);
	Context2Object obj2(logger2);

	view->connectLogger(logger1);
	view->connectLogger(logger2);

	

	app.exec();
	getchar();
	return 0;
}