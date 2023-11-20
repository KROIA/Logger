#include "Context2Object.h"
#include "Logger.h"

Context2Object::Context2Object(Log::ContextLogger& logger, QWidget*parent)
	: ContextObject("Context1Object", logger, parent)
{
	m_messageTimer.setInterval(2000);
	m_warningTimer.setInterval(4000);
	m_errorTimer.setInterval(8000);


}

Context2Object::~Context2Object()
{

}

void Context2Object::on_createContext_pushButton_clicked()
{
	ContextObject::on_createContext_pushButton_clicked();
	ContextObject* child = m_contextObjects[m_contextObjects.size() - 1];
	Log::QContextLoggerTreeView* view = new Log::QContextLoggerTreeView();
	view->connectLogger(*(child->m_logger));
	view->show();
}
