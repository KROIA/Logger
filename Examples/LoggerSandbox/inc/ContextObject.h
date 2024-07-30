#pragma once

#include <QWidget>
#include "ui_ContextObject.h"
#include "Logger.h"
#include <QTimer>

class ContextObject : public QWidget
{
	Q_OBJECT

public:
	ContextObject(
		const std::string& contextName,
		Log::LogObject& logger,
		Log::UI::QTreeConsoleView* view,
		Log::UI::QConsoleView* consoleView,
		QWidget *parent = nullptr);
	virtual ~ContextObject();


	Log::LogObject* m_logger;

protected slots:
	void onMessageTimer();
	void onWarningTimer();
	void onErrorTimer();

	void on_clear_pushButton_clicked();
	virtual void on_createContext_pushButton_clicked();
	void onCloseContext_pushButton_clicked();
	void onDeleteContext_pushButton_clicked();
	void onDateTimeFormatSwitch_pushButton_clicked();

	//void onDelete(Log::Logger::AbstractLogger& logger);
protected:

	QTimer m_messageTimer;
	QTimer m_warningTimer;
	QTimer m_errorTimer;
	
	unsigned int m_counter;

	std::vector< ContextObject*> m_contextObjects;
private:
	Ui::ContextObject ui;
	size_t m_instanceID;
	Log::UI::QTreeConsoleView* m_view = nullptr;
	Log::UI::QConsoleView* m_consoleView = nullptr;
	static size_t s_instanceCounter;
	int counter = 0;
	


};