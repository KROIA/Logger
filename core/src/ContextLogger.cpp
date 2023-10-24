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

	ContextLogger::ContextID ContextLogger::createContext(const Message& contextMsg)
	{
		ContextMessage* c = new ContextMessage{ (ContextID)m_messages.size(), contextMsg };
		c->contextInfo.setTabCount(m_tabCount);
		m_messages.push_back(c);
		return c->contextID;
	}
	bool ContextLogger::switchContext(ContextID contextID)
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
	ContextLogger::ContextID ContextLogger::getCurrentContextID() const
	{
		return m_currentContextID;
	}
	size_t ContextLogger::getContextCount() const
	{
		return m_messages.size();
	}

	const ContextLogger::ContextMessage& ContextLogger::getContextMessage(ContextID contextID) const
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
	std::vector<ContextLogger::ContextMessage> ContextLogger::getContextMessages() const
	{
		std::vector<ContextMessage> msgs;
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


	std::ostream& operator<<(std::ostream& os, const ContextLogger::ContextMessage& msg)
	{
		os << msg.contextInfo.getDateTime().toString() + " " + msg.contextInfo.getLevelString() + " " + msg.contextInfo.getText() + "\n";
		for (const Message& m : msg.messages)
		{
			std::string timeDate = m.getDateTime().toString();
			std::string level = m.getLevelString();
			std::string text = m.getText();

			os << " " + timeDate + " " + level + " " + text + "\n";
		}
		return os;
	}
	void ContextLogger::ContextMessage::toStringVector(std::vector<std::string>& list) const
	{
		std::string timeDate = contextInfo.getDateTime().toString();
		std::string level = contextInfo.getLevelString();
		std::string text = contextInfo.getText();

		list.push_back(timeDate + std::string(contextInfo.getTabCount(), ' ') + level + text);
		for (const Message& m : messages)
		{
			std::string timeDate = m.getDateTime().toString();
			std::string level = m.getLevelString();
			std::string text = m.getText();

			list.push_back(timeDate + std::string(m.getTabCount() + 1, ' ') + level + text);
		}
	}

	std::ostream& operator<<(std::ostream& os, const ContextLogger& context)
	{
		std::vector<std::string> lines;
		context.toStringVector(lines);
		for (const std::string& l : lines)
			os << l << "\n";
		return os;
	}
	void ContextLogger::toStringVector(std::vector<std::string>& list) const
	{
		
		list.push_back("Context: " + m_name);

		for (ContextMessage* m : m_messages)
		{
			std::vector<std::string> tmp;
			m->toStringVector(tmp);

			list.insert(list.end(), tmp.begin(), tmp.end());
		}
	}
}