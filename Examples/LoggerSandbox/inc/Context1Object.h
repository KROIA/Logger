#pragma once

#include "ContextObject.h"

class Context1Object : public ContextObject
{
	Q_OBJECT
public:
	Context1Object(Log::ContextLogger& logger, QWidget*parent = nullptr);
	~Context1Object();



private:

};
