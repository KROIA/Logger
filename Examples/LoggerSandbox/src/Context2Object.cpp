#include "Context2Object.h"
#include "Logger.h"

Context2Object::Context2Object(Log::LogObject& logger, 
	Log::UI::QContextLoggerTreeView* view, 
	Log::UI::QConsoleView* consoleView,
	QWidget*parent)
	: ContextObject("Context1Object", logger, view, consoleView, parent)
{
	m_messageTimer.setInterval(2000);
	m_warningTimer.setInterval(4000);
	m_errorTimer.setInterval(8000);
	m_logger->setColor(Log::Color(100,100,255));

}

Context2Object::~Context2Object()
{

}

void Context2Object::on_createContext_pushButton_clicked()
{
	ContextObject::on_createContext_pushButton_clicked();
	ContextObject* child = m_contextObjects[m_contextObjects.size() - 1];
	Log::UI::QContextLoggerTreeView* view = new Log::UI::QContextLoggerTreeView();
	//view->attachLogger(*(child->m_logger));
	view->show();
}