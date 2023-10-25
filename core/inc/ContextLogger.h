#pragma once

#include "AbstractLogger.h"
#include <vector>
#include <sstream>


namespace Log
{
	class LOGGER_EXPORT ContextLogger : public AbstractLogger
	{
	public:
	    typedef unsigned int ContextID;

		struct LOGGER_EXPORT ContextMessage
		{
			ContextID contextID;
			Message contextInfo;
			std::vector<Message> messages;

			void toStringVector(std::vector<std::string>& list) const;
			friend std::ostream& operator<<(std::ostream& os, const ContextMessage& msg);
		};
		

		ContextLogger(const std::string& name = "");
		ContextLogger(const ContextLogger& other);

		~ContextLogger();

		ContextID createContext(const Message& contextMsg);
		bool switchContext(ContextID contextID);
		ContextID getCurrentContextID() const;
		size_t getContextCount() const;

		const ContextMessage& getContextMessage(ContextID contextID) const;
		std::vector<ContextMessage> getContextMessages() const;

		void clear();

		void toStringVector(std::vector<std::string>& list) const;
		friend std::ostream& operator<<(std::ostream& os, const ContextLogger& msg);

	private:
		void logInternal(const Message& msg) override;

		std::vector<ContextMessage*> m_messages;
		//std::vector<ContextLogger*> m_childs;
		
		ContextMessage* m_currentContext;
		unsigned int m_currentContextID;
	};
}