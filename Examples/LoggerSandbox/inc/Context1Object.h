#pragma once

#include "ContextObject.h"

class Context1Object : public ContextObject
{
	Q_OBJECT
public:
	Context1Object(Log::Logger::ContextLogger& logger, QWidget*parent = nullptr);
	~Context1Object();



private:

};
