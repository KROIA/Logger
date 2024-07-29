#pragma once

#include "Logger_base.h"
#include "LogManager.h"
#include <QObject>

namespace Log
{
	class AbstractReceiver;
	// Encapsulating signal handling in a private class
	class SignalReceiver : public QObject
	{
		friend class AbstractReceiver;
		Q_OBJECT
	public:
		SignalReceiver(AbstractReceiver* receiver);
		~SignalReceiver() {};

	public slots:
		void onNewLogger(LogObject::Info loggerInfo);
		void onLoggerInfoChanged(LogObject::Info info);
		void onLogMessage(Message message);
		void onChangeParent(LoggerID childID, LoggerID newParentID);

	private:
		AbstractReceiver* receiver;
	};

	class AbstractReceiver
	{
		friend class SignalReceiver;
	public:
		AbstractReceiver();
		virtual ~AbstractReceiver();

	protected:
		virtual void onNewLogger(LogObject::Info loggerInfo) = 0;
		virtual void onLoggerInfoChanged(LogObject::Info info) = 0;
		virtual void onLogMessage(Message message) = 0;
		virtual void onChangeParent(LoggerID childID, LoggerID newParentID) = 0;
	private:
		


		

		SignalReceiver signalReceiver;
	};

	
}