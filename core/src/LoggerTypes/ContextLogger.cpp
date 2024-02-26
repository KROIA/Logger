#include "LoggerTypes/ContextLogger.h"
#include <sstream>

namespace Log
{
	namespace Logger
	{
		DEFINE_SIGNAL_CONNECT_DISCONNECT(ContextLogger, onContextCreate, ContextLogger&);
		DEFINE_SIGNAL_CONNECT_DISCONNECT(ContextLogger, onContextDestroy, ContextLogger&);


		ContextLogger::ContextLogger(const std::string& name, ContextLogger* parent) 
			: AbstractLogger(name)
			, m_parent(parent)
			, m_rootParent(parent ? parent->m_rootParent : this)
			, onContextCreate("onContextCreate")
			, onContextDestroy("onContextDestroy")
		{
			if (!m_parent)
				getAllRootLoggers().push_back(this);
		}

		ContextLogger::ContextLogger(const std::string& name) 
			: AbstractLogger(name)
			, m_parent(nullptr)
			, m_rootParent(this)
			, onContextCreate("onContextCreate")
			, onContextDestroy("onContextDestroy")
		{
			getAllRootLoggers().push_back(this);
		}
		ContextLogger::ContextLogger(const ContextLogger& other) 
			: AbstractLogger(other)
			, m_parent(nullptr)
			, m_rootParent(this)
			, onContextCreate("onContextCreate")
			, onContextDestroy("onContextDestroy")
		{
			m_childs.reserve(other.m_childs.size());
			for (size_t i = 0; i < other.m_childs.size(); ++i)
			{
				m_childs.push_back(new ContextLogger(*other.m_childs[i]));
			}
		}

		ContextLogger::~ContextLogger()
		{
			destroyAllContext();
			if (!m_parent)
			{
				std::vector<ContextLogger*>& list = getAllRootLoggers();
				auto it = std::find(list.begin(), list.end(), this);
				if (it != list.end()) {
					list.erase(it);
				}
			}
			else
			{
				auto it = std::find(m_parent->m_childs.begin(), m_parent->m_childs.end(), this);
				if (it != m_parent->m_childs.end()) {
					m_parent->m_childs.erase(it);
				}
			}
		}

		ContextLogger* ContextLogger::getParent() const
		{
			return m_parent;
		}
		ContextLogger* ContextLogger::getRootParent() const
		{
			return m_rootParent;
		}
		const std::vector<ContextLogger*>& ContextLogger::getAllLogger()
		{
			return getAllRootLoggers();
		}

		ContextLogger* ContextLogger::createContext(const std::string& name)
		{
			ContextLogger* context = new ContextLogger(name, this);
			context->tabIn();
			m_childs.push_back(context);
			emitRecursive_onContextCreate(*context);
			return context;
		}

		void ContextLogger::clear()
		{
			for (ContextLogger* c : m_childs)
				c->clear();
			AbstractLogger::clear();
		}
		void ContextLogger::destroyAllContext()
		{
			std::vector<ContextLogger*> childs = m_childs;
			m_childs.clear();
			for (size_t i = 0; i < childs.size(); ++i)
			{
				childs[i]->destroyAllContext();
				emitRecursive_onContextDestroy(*childs[i]);
				delete childs[i];
			}
		}
		void ContextLogger::destroyContext(ContextLogger* child)
		{
			auto it = std::find(m_childs.begin(), m_childs.end(), child);
			if (it == m_childs.end())
				return;
			m_childs.erase(it);
			child->destroyAllContext();
			emitRecursive_onContextDestroy(*child);
			delete child;
		}

		std::vector<ContextLogger*>& ContextLogger::getAllRootLoggers()
		{
			static std::vector<ContextLogger*> allRootLoggers;
			return allRootLoggers;
		}


		std::ostream& operator<<(std::ostream& os, const ContextLogger& msg)
		{
			std::vector<std::string> lines;
			msg.toStringVector(lines, DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond);
			for (size_t i = 0; i < lines.size(); ++i)
				os << lines[i] << "\n";

			return os;
		}

		void ContextLogger::getMessagesRecursive(std::vector<Message>& list) const
		{
			const std::vector<Message>& messages = getMessages();
			list.insert(list.end(), messages.begin(), messages.end());
			for (const ContextLogger* m : m_childs)
			{
				m->getMessagesRecursive(list);
			}
		}
		const std::vector<ContextLogger*>& ContextLogger::getChilds() const
		{
			return m_childs;
		}

		void ContextLogger::toStringVector(std::vector<std::string>& list, DateTime::Format dateTimeFormat) const
		{
			toStringVector(0, list, dateTimeFormat);
		}

		void ContextLogger::toStringVector(size_t depth, std::vector<std::string>& list, DateTime::Format dateTimeFormat) const
		{
			std::string depthStr(depth, ' ');
			const std::vector<Message>& messages = getMessages();
			std::string title = getCreationDateTime().toString(dateTimeFormat) + depthStr +
				"Context: " + getName() +
				" Msgs: " + std::to_string(messages.size()) +
				" Childs: " + std::to_string(m_childs.size());
			list.push_back(title);
			for (const Message& m : messages)
			{
				std::string msgString = m.getDateTime().toString(dateTimeFormat) + " " +
					Utilities::getLevelStr(m.getLevel()) +
					depthStr +
					std::string(m.getTabCount(), ' ') +
					m.getText();
				list.push_back(msgString);
			}

			for (const ContextLogger* m : m_childs)
			{
				std::vector<std::string> tmp;
				m->toStringVector(depth + 1, tmp, dateTimeFormat);

				list.insert(list.end(), tmp.begin(), tmp.end());
			}
		}




		void ContextLogger::emitRecursive_onContextCreate(ContextLogger& newContext)
		{
			if (onContextCreate.getSlotCount())
				onContextCreate.emitSignal(newContext);
			if (m_parent)
				m_parent->emitRecursive_onContextCreate(newContext);
		}
		void ContextLogger::emitRecursive_onContextDestroy(ContextLogger& destroyedContext)
		{
			if (onContextDestroy.getSlotCount())
				onContextDestroy.emitSignal(destroyedContext);
			if (m_parent)
				m_parent->emitRecursive_onContextDestroy(destroyedContext);
		}
	}
}