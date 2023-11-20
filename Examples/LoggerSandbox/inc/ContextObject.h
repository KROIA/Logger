#pragma once

#include <QWidget>
#include "ui_ContextObject.h"
#include "ContextLogger.h"
#include <QTimer>

class ContextObject : public QWidget
{
	Q_OBJECT

public:
	ContextObject(
		const std::string& contextName,
		Log::ContextLogger& logger, 
		QWidget *parent = nullptr);
	~ContextObject();


	Log::ContextLogger* m_logger;

protected slots:
	void onMessageTimer();
	void onWarningTimer();
	void onErrorTimer();

	void on_clear_pushButton_clicked();
	virtual void on_createContext_pushButton_clicked();
	void onCloseContext_pushButton_clicked();
	void onDeleteContext_pushButton_clicked();

	void onDelete(Log::ContextLogger& logger);
protected:

	QTimer m_messageTimer;
	QTimer m_warningTimer;
	QTimer m_errorTimer;
	
	unsigned int m_counter;

	std::vector< ContextObject*> m_contextObjects;
private:
	Ui::ContextObject ui;
	size_t m_instanceID;
	static size_t s_instanceCounter;


};
