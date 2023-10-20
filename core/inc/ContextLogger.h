#pragma once

#include "AbstractLogger.h"
#include <vector>

namespace Log
{
	class LOGGER_EXPORT ContextLogger : public AbstractLogger
	{
	public:
		struct LOGGER_EXPORT ContextMessage
		{
			unsigned int contextID;
			Message contextInfo;
			std::vector<Message> messages;
		};

		ContextLogger(const std::string& name = "");
		ContextLogger(const ContextLogger& other);

		~ContextLogger();

		unsigned int createContext(const Message& contextMsg);
		bool switchContext(unsigned int contextID);
		unsigned int getCurrentContextID() const;
		unsigned int getContextCount() const;

		const ContextMessage& getContextMessage(unsigned int contextID) const;
		std::vector < ContextMessage> getContextMessages() const;

		void clear();

	private:
		void logInternal(const Message& msg) override;

		std::vector<ContextMessage*> m_messages;

		
		ContextMessage* m_currentContext;
		unsigned int m_currentContextID;
		//NastedMessage m_currentContext;
	};
}