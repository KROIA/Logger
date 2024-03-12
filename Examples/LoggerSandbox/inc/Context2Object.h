#pragma once

#include "ContextObject.h"

class Context2Object : public ContextObject
{
	Q_OBJECT

public:
	Context2Object(Log::Logger::ContextLogger& logger, 
		Log::UI::QContextLoggerTreeView* view, 
		Log::UI::QConsoleView* consoleView,
		QWidget *parent = nullptr);
	~Context2Object();

protected slots:
	void on_createContext_pushButton_clicked() override;
};