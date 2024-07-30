#include "ContextObject.h"
#include <QLabel>
#include <QDebug>
#include <QIcon>
#include <QToolButton>

size_t ContextObject::s_instanceCounter = 0;
ContextObject::ContextObject(
	const std::string& contextName,
	Log::LogObject& logger,
	Log::UI::QTreeConsoleView* view,
	Log::UI::QConsoleView* consoleView,
	QWidget* parent)
	: QWidget(parent)
	, m_logger(new Log::LogObject(logger, contextName))
	, m_view(view)
	, m_consoleView(consoleView)
{
	ui.setupUi(this);

	//m_logger->connect_onDelete_slot(this, &ContextObject::onDelete);
	m_logger->setColor(Log::Color::darkGray);

	m_messageTimer.setInterval(1000);
	m_warningTimer.setInterval(2000);
	m_errorTimer.setInterval(4000);
	connect(&m_messageTimer, &QTimer::timeout, this, &ContextObject::onMessageTimer);
	connect(&m_warningTimer, &QTimer::timeout, this, &ContextObject::onWarningTimer);
	connect(&m_errorTimer, &QTimer::timeout, this, &ContextObject::onErrorTimer);
	connect(ui.deleteContext_pushButton, &QPushButton::clicked, this, &ContextObject::onDeleteContext_pushButton_clicked);
	connect(ui.closeContext_pushButton, &QPushButton::clicked, this, &ContextObject::onCloseContext_pushButton_clicked);
	connect(ui.dateTimeFormatSwitch_pushButton, &QPushButton::clicked, this, &ContextObject::onDateTimeFormatSwitch_pushButton_clicked);
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
		//m_logger->disconnect_onDelete_slot(this, &ContextObject::onDelete);
		m_logger->log("Destroy " + m_logger->getName());
	}

}


void ContextObject::onMessageTimer()
{
	++m_counter;
	if (m_logger)
	{
		if(counter<10)
			m_logger->log(std::to_string(m_counter) + " Message from " + m_logger->getName());
		else
		{
			std::string txt = "Long Message from " + m_logger->getName() + "\n";
			txt += "This is a new line\n";
			txt += "This is another new line\n";
			m_logger->log(txt, Log::Level::info);
			counter = 0;
		}
		++counter;
	}
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
	//if (m_logger)
	//m_logger->clear();
}
void ContextObject::on_createContext_pushButton_clicked()
{
	ContextObject *obj = new ContextObject("ContextObject_" + std::to_string(m_contextObjects.size()), *m_logger, m_view, m_consoleView);

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
		Log::LogObject *logger = obj->m_logger;
		obj->m_logger = nullptr;
		obj->deleteLater();
		
		
		obj->m_messageTimer.stop();
		obj->m_warningTimer.stop();
		obj->m_errorTimer.stop();
		//if(logger)
		//	if(logger->getParent())
		//		logger->getParent()->destroyContext(logger);
	}
}
void ContextObject::onDateTimeFormatSwitch_pushButton_clicked()
{
	if(!m_view)
		return;
	static bool switcher = false;
	switcher = !switcher;
	Log::DateTime::Format format = Log::DateTime::Format::dayMonthYear | Log::DateTime::Format::hourMinuteSecondMillisecond;
	if (switcher)
		format = Log::DateTime::Format::yearMonthDay | Log::DateTime::Format::hourMinuteSecond;
	m_view->setDateTimeFormat(format);
	if (m_consoleView)
		m_consoleView->setDateTimeFormat(format);
}
/*
void ContextObject::onDelete(Log::LogObject& logger)
{
	//qDebug() << "Logger deleted";
	m_logger = nullptr;
	m_messageTimer.stop();
	m_warningTimer.stop();
	m_errorTimer.stop();
	hide();
}*/