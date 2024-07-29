#include "ReceiverTypes/AbstractReceiver.h"
#include "LogManager.h"

namespace Log
{
	SignalReceiver::SignalReceiver(AbstractReceiver* receiver)
		: receiver(receiver)
	{
		LogManager& m = LogManager::instance();
		connect(&m, &LogManager::onNewLogger, this, &SignalReceiver::onNewLogger, Qt::QueuedConnection);
		connect(&m, &LogManager::onLoggerInfoChanged, this, &SignalReceiver::onLoggerInfoChanged, Qt::QueuedConnection);
		connect(&m, &LogManager::onLogMessage, this, &SignalReceiver::onLogMessage, Qt::QueuedConnection);
		connect(&m, &LogManager::onChangeParent, this, &SignalReceiver::onChangeParent, Qt::QueuedConnection);
	}

	void SignalReceiver::onNewLogger(LogObject::Info loggerInfo) 
	{ 
		receiver->onNewLogger(loggerInfo);
	}
	void SignalReceiver::onLoggerInfoChanged(LogObject::Info info) { receiver->onLoggerInfoChanged(info); }
	void SignalReceiver::onLogMessage(Message message) { receiver->onLogMessage(message); }
	void SignalReceiver::onChangeParent(LoggerID childID, LoggerID newParentID) { receiver->onChangeParent(childID, newParentID); }




	AbstractReceiver::AbstractReceiver()
		: signalReceiver(this)
	{
		
	}
	AbstractReceiver::~AbstractReceiver()
	{

	}



	
}