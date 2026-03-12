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
	};
}