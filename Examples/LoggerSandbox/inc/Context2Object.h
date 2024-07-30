#pragma once

#include "ContextObject.h"

class Context2Object : public ContextObject
{
	Q_OBJECT

public:
	Context2Object(Log::LogObject& logger,
		Log::UI::QTreeConsoleView* view, 
		Log::UI::QConsoleView* consoleView,
		QWidget *parent = nullptr);
	~Context2Object();

protected slots:
	void on_createContext_pushButton_clicked() override;
};