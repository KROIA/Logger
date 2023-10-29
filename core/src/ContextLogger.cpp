#include "ContextLogger.h"


namespace Log
{
	ContextLogger::ContextLogger(const std::string& name)
		: AbstractLogger(name)
	{

	}
	ContextLogger::ContextLogger(const ContextLogger& other)
		: AbstractLogger(other)
	{
		m_messages.reserve(other.m_messages.size());
		for (size_t i = 0; i < other.m_messages.size(); ++i)
		{
			m_messages.push_back(other.m_messages[i]);
		}

		m_childs.reserve(other.m_childs.size());
		for (size_t i=0; i<other.m_childs.size(); ++i)
		{
			m_childs.push_back(new ContextLogger(*other.m_childs[i]));
		}
	}

	ContextLogger::~ContextLogger()
	{
		destroyChilds();
		clear();
	}

	ContextLogger* ContextLogger::createContext(const std::string& name)
	{
		ContextLogger* context = new ContextLogger(name);
		m_childs.push_back(context);
		return context;
	}


	void ContextLogger::clear()
	{		
		m_messages.clear();
		for (ContextLogger* c : m_childs)
			c->clear();
	}
	void ContextLogger::destroyChilds()
	{
		for (ContextLogger* c : m_childs)
			delete c;
		m_childs.clear();
	}


	void ContextLogger::logInternal(const Message& msg)
	{
		m_messages.push_back(msg);
	}


	std::ostream& operator<<(std::ostream& os, const ContextLogger& msg)
	{
		std::vector<std::string> lines;
		msg.toStringVector(lines);
		for (size_t i = 0; i < lines.size(); ++i)
			os << lines[i] << "\n";

		return os;
	}

	void ContextLogger::toStringVector(std::vector<std::string>& list) const
	{
		toStringVector(0, list);
	}

	void ContextLogger::toStringVector(size_t depth, std::vector<std::string>& list) const
	{
		std::string depthStr(depth, ' ');

		std::string title = m_creationsTime.toString() + depthStr +
			"Context: " + m_name +
			" Msgs: " + std::to_string(m_messages.size()) +
			" Childs: " + std::to_string(m_childs.size());
		list.push_back(title);
		for (const Message& m : m_messages)
		{
			std::string msgString = m.getDateTime().toString() + " " +
				getLevelStr(m.getLevel()) +
				depthStr +
				std::string(m.getTabCount(), ' ') +
				m.getText();
		}

		for (const ContextLogger* m : m_childs)
		{
			std::vector<std::string> tmp;
			m->toStringVector(depth + 1 ,tmp);

			list.insert(list.end(), tmp.begin(), tmp.end());
		}
	}
}