#include "Context1Object.h"

Context1Object::Context1Object(Log::ContextLogger& logger, QWidget *parent)
	: ContextObject("Context1Object", logger, parent)
{
	m_messageTimer.setInterval(1000);
	m_warningTimer.setInterval(2000);
	m_errorTimer.setInterval(4000);
}

Context1Object::~Context1Object()
{

}


