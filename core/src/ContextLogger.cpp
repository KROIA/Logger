#include "ContextLogger.h"


namespace Log
{
	ContextLogger::ContextLogger(const std::string& name)
		: AbstractLogger(name)
		, m_currentContext(nullptr)
		, m_currentContextID(0)
	{

	}
	ContextLogger::ContextLogger(const ContextLogger& other)
		: AbstractLogger(other)
		, m_currentContext(nullptr)
		, m_currentContextID(other.m_currentContextID)
	{
		m_messages.reserve(other.m_messages.size());
		for (size_t i=0; i<other.m_messages.size(); ++i)
		{
			m_messages.push_back(new ContextMessage(*other.m_messages[i]));
		}
		switchContext(m_currentContextID);
	}

	ContextLogger::~ContextLogger()
	{
		clear();
	}

	unsigned int ContextLogger::createContext(const Message& contextMsg)
	{
		m_messages.push_back(new ContextMessage{(unsigned int) m_messages.size(), contextMsg });
		return m_messages.size() - 1;
	}
	bool ContextLogger::switchContext(unsigned int contextID)
	{
		for (ContextMessage* m : m_messages)
		{
			if (m->contextID == contextID)
			{
				m_currentContextID = contextID;
				m_currentContext = m;
				return true;
			}
		}
		return false;
	}
	unsigned int ContextLogger::getCurrentContextID() const
	{
		return m_currentContextID;
	}
	unsigned int ContextLogger::getContextCount() const
	{
		return (unsigned int)m_messages.size();
	}

	const ContextLogger::ContextMessage& ContextLogger::getContextMessage(unsigned int contextID) const
	{
		for (ContextMessage* m : m_messages)
		{
			if (m->contextID == contextID)
			{
				return *m;
			}
		}
		static const ContextLogger::ContextMessage dummy{ 0, Message("dummy") };
		return dummy;
	}
	std::vector < ContextLogger::ContextMessage> ContextLogger::getContextMessages() const
	{
		std::vector <ContextMessage> msgs;
		msgs.reserve(m_messages.size());
		for (ContextMessage* m : m_messages)
		{
			msgs.push_back(*m);
		}
		return msgs;
	}

	void ContextLogger::clear()
	{
		m_currentContextID = 0;
		m_currentContext = nullptr;
		for (ContextMessage* m : m_messages)
			delete m;
		m_messages.clear();
	}


	void ContextLogger::logInternal(const Message& msg)
	{
		if (!m_currentContext)
		{
			unsigned int def = createContext(Log::Message(m_name + " default context"));
			switchContext(def);
		}
		m_currentContext->messages.push_back(msg);
	}
}