#pragma once

#include "ContextObject.h"

class Context1Object : public ContextObject
{
	Q_OBJECT
public:
	Context1Object(Log::LogObject& logger, 
		Log::UI::QTreeConsoleView* view, 
		Log::UI::QConsoleView* consoleView,
		QWidget*parent = nullptr);
	~Context1Object();



private:

};