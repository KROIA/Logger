#include "Context1Object.h"

Context1Object::Context1Object(Log::Logger::ContextLogger& logger, Log::UI::QContextLoggerTreeView* view, QWidget *parent)
	: ContextObject("Context1Object", logger, view, parent)
{
	m_messageTimer.setInterval(1000);
	m_warningTimer.setInterval(2000);
	m_errorTimer.setInterval(4000);
	m_logger->setColor(Log::Color::green);
}

Context1Object::~Context1Object()
{

}


