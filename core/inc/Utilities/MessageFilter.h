#pragma once

#include "Logger_base.h"


namespace Log
{
	class Message;
	class LOGGER_API MessageFilter
	{
	public:
			MessageFilter() = default;
			virtual ~MessageFilter() = default;
			
			/**
			* @return true if the message should be accepted by the receiver,
			*         false if it should be filtered out
			*/
			virtual bool filter(const Message& message) const = 0;

			/**
			* @return true if a lifecycle event (new logger / info change /
			*         reparent) for this logger id should be accepted.
			* Default accepts everything so non-id filters keep prior behaviour.
			*/
			virtual bool filterByLoggerID(LoggerID id) const { (void)id; return true; }
	};
}