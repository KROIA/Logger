#pragma once

#include "ContextObject.h"

class Context2Object : public ContextObject
{
	Q_OBJECT

public:
	Context2Object(Log::ContextLogger& logger, QWidget *parent = nullptr);
	~Context2Object();

protected slots:
	void on_createContext_pushButton_clicked() override;
};
