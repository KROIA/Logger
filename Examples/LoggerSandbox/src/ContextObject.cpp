#include "ContextObject.h"
#include <QLabel>
#include <QDebug>
#include <QIcon>
#include <QToolButton>

size_t ContextObject::s_instanceCounter = 0;
ContextObject::ContextObject(
	const std::string& contextName,
	Log::Logger::ContextLogger& logger,
	QWidget* parent)
	: QWidget(parent)
	, m_logger(logger.createContext(contextName))
{
	ui.setupUi(this);

	m_logger->connect_onDelete_slot(this, &ContextObject::onDelete);

	m_messageTimer.setInterval(1000);
	m_warningTimer.setInterval(2000);
	m_errorTimer.setInterval(4000);
	connect(&m_messageTimer, &QTimer::timeout, this, &ContextObject::onMessageTimer);
	connect(&m_warningTimer, &QTimer::timeout, this, &ContextObject::onWarningTimer);
	connect(&m_errorTimer, &QTimer::timeout, this, &ContextObject::onErrorTimer);
	m_messageTimer.start();
	m_warningTimer.start();
	m_errorTimer.start();

	m_counter = 0;
	m_instanceID = s_instanceCounter;
	s_instanceCounter++;

	

	this->show();
}

ContextObject::~ContextObject()
{	
	m_messageTimer.stop();
	m_warningTimer.stop();
	m_errorTimer.stop();
	if (m_logger)
	{
		m_logger->disconnect_onDelete_slot(this, &ContextObject::onDelete);
		m_logger->log("Destroy " + m_logger->getName());
	}

}


void ContextObject::onMessageTimer()
{
	++m_counter;
	if (m_logger)
	m_logger->log(std::to_string(m_counter) + " Message from " + m_logger->getName());
}
void ContextObject::onWarningTimer()
{
	++m_counter;
	if (m_logger)
	m_logger->log(std::to_string(m_counter) + " Warning from " + m_logger->getName(), Log::Level::warning);
}
void ContextObject::onErrorTimer()
{
	++m_counter;
	if (m_logger)
	m_logger->log(std::to_string(m_counter) + " Error from " + m_logger->getName(), Log::Level::error);
}

void ContextObject::on_clear_pushButton_clicked()
{
	if (m_logger)
	m_logger->clear();
}
void ContextObject::on_createContext_pushButton_clicked()
{
	ContextObject *obj = new ContextObject("ContextObject_" + std::to_string(m_contextObjects.size()), *m_logger);

	connect(obj->ui.closeContext_pushButton, &QPushButton::clicked, this, &ContextObject::onCloseContext_pushButton_clicked);
	connect(obj->ui.deleteContext_pushButton, &QPushButton::clicked, this, &ContextObject::onDeleteContext_pushButton_clicked);
	m_contextObjects.push_back(obj);
}
void ContextObject::onCloseContext_pushButton_clicked()
{
	ContextObject* obj = dynamic_cast<ContextObject*>(sender()->parent());
	if (obj)
	{
		obj->close();
		delete obj;
	}
}
void ContextObject::onDeleteContext_pushButton_clicked()
{
	ContextObject* obj = dynamic_cast<ContextObject*>(sender()->parent());
	if (obj)
	{
		obj->close();
		Log::Logger::ContextLogger *logger = obj->m_logger;
		obj->m_logger = nullptr;
		obj->deleteLater();
		
		
		obj->m_messageTimer.stop();
		obj->m_warningTimer.stop();
		obj->m_errorTimer.stop();
		if(logger)
			if(logger->getParent())
				logger->getParent()->destroyContext(logger);
	}
}

void ContextObject::onDelete(Log::Logger::AbstractLogger& logger)
{
	qDebug() << "Logger deleted";
	m_logger = nullptr;
	m_messageTimer.stop();
	m_warningTimer.stop();
	m_errorTimer.stop();
	hide();
}