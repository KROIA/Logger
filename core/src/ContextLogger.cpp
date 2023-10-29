#include "ContextLogger.h"
#ifdef LOGGER_QT
	#include "QT/QContextLoggerTreeWidgetItem.h"
#endif

namespace Log
{
	std::vector<ContextLogger*> ContextLogger::s_allRootLoggers;

	ContextLogger::ContextLogger(const std::string& name, ContextLogger* parent)
		: AbstractLogger(name)
		, m_parent(parent)
#ifdef LOGGER_QT
		, m_treeWidget(nullptr)
#endif
	{
		s_allRootLoggers.push_back(this);
	}

	ContextLogger::ContextLogger(const std::string& name)
		: AbstractLogger(name)
		, m_parent(nullptr)
#ifdef LOGGER_QT
		, m_treeWidget(nullptr)
#endif
	{
		s_allRootLoggers.push_back(this);
	}
	ContextLogger::ContextLogger(const ContextLogger& other)
		: AbstractLogger(other)
		, m_parent(nullptr)
		, m_creationsTime(other.m_creationsTime)
#ifdef LOGGER_QT
		, m_treeWidget(nullptr)
#endif
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
		if (!m_parent)
		{
			auto it = std::find(s_allRootLoggers.begin(), s_allRootLoggers.end(), this);
			if (it != s_allRootLoggers.end()) {
				s_allRootLoggers.erase(it);
			}
		}
	}

	const std::vector<ContextLogger*>& ContextLogger::getAllLogger()
	{
		return s_allRootLoggers;
	}

	ContextLogger* ContextLogger::createContext(const std::string& name)
	{
		ContextLogger* context = new ContextLogger(name, this);
		m_childs.push_back(context);
#ifdef LOGGER_QT
		if (m_treeWidget)
		{
			m_treeWidget->addChild(context->getTreeWidgetItem());
		}
#endif
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

	const DateTime& ContextLogger::getDateTime() const
	{
		return m_creationsTime;
	}
	const std::vector<Message>& ContextLogger::getMessages() const
	{
		return m_messages;
	}
	const std::vector<ContextLogger*>& ContextLogger::getChilds() const
	{
		return m_childs;
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
			list.push_back(msgString);
		}

		for (const ContextLogger* m : m_childs)
		{
			std::vector<std::string> tmp;
			m->toStringVector(depth + 1 ,tmp);

			list.insert(list.end(), tmp.begin(), tmp.end());
		}
	}


#ifdef LOGGER_QT
	QContextLoggerTreeWidgetItem* ContextLogger::getTreeWidgetItem() 
	{
		if (!m_treeWidget)
		{
			m_treeWidget = new QContextLoggerTreeWidgetItem();
			for (size_t i = 0; i < m_childs.size(); ++i)
			{
				m_treeWidget->addChild(m_childs[i]->getTreeWidgetItem());
			}
			updateTreeWidgetItem();
		}
		return m_treeWidget;
	}

	void ContextLogger::updateTreeWidgetItem()
	{
		m_treeWidget->updateData(*this);
		for (size_t i = 0; i < m_childs.size(); ++i)
		{
			m_childs[i]->updateTreeWidgetItem();
		}
	}
#endif
}