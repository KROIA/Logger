#include "ContextLogger.h"
#ifdef LOGGER_QT
	#include "QT/QContextLoggerTree.h"
#endif

namespace Log
{
	std::vector<ContextLogger*> ContextLogger::s_allRootLoggers;

	DEFINE_SIGNAL_CONNECT_DISCONNECT(ContextLogger, onNewMessage, const ContextLogger&, const Message&);
	DEFINE_SIGNAL_CONNECT_DISCONNECT(ContextLogger, onNewContext, const ContextLogger&, const ContextLogger&);
	DEFINE_SIGNAL_CONNECT_DISCONNECT(ContextLogger, onDestroyContext, const ContextLogger&);
	DEFINE_SIGNAL_CONNECT_DISCONNECT(ContextLogger, onClear, const ContextLogger&);
	DEFINE_SIGNAL_CONNECT_DISCONNECT(ContextLogger, onDelete, ContextLogger&);


	ContextLogger::ContextLogger(const std::string& name, ContextLogger* parent) :
//#ifdef LOGGER_QT
//		QObject(parent), 
//#endif
		  AbstractLogger(name)
		, m_parent(parent)
		, m_rootParent(parent ? parent->m_rootParent : this)
		, onNewContext("onNewContext")
		, onDestroyContext("onDestroyContext")
		, onNewMessage("onNewMessage")
		, onClear("onClear")
		, onDelete("onDelete")

//#ifdef LOGGER_QT
	//	, m_subscribedUITree(nullptr)
//#endif
	{
		if (!m_parent)
		{
			s_allRootLoggers.push_back(this);
		}
		else
		{
			//subscribeToUI(m_parent->m_subscribedUITree);
		}	
	}

	ContextLogger::ContextLogger(const std::string& name):
//#ifdef LOGGER_QT
//		QObject(nullptr),
//#endif
		  AbstractLogger(name)
		, m_parent(nullptr)
		, m_rootParent(this)
		, onNewContext("onNewContext")
		, onDestroyContext("onDestroyContext")
		, onNewMessage("onNewMessage")
		, onClear("onClear")
		, onDelete("onDelete")
//#ifdef LOGGER_QT
	//	, m_subscribedUITree(nullptr)
//#endif
	{
		s_allRootLoggers.push_back(this);
	}
	ContextLogger::ContextLogger(const ContextLogger& other):
//#ifdef LOGGER_QT
//		QObject(nullptr),
//#endif
		  AbstractLogger(other)
		, m_parent(nullptr)
		, m_rootParent(this)
		, m_creationsTime(other.m_creationsTime)
		, onNewContext("onNewContext")
		, onDestroyContext("onDestroyContext")
		, onNewMessage("onNewMessage")
		, onClear("onClear")
		, onDelete("onDelete")
//#ifdef LOGGER_QT
	//	, m_subscribedUITree(nullptr)
//#endif
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
		// onNewContext.disconnectAll();
		// onNewMessage.disconnectAll();
		// onDestroyContext.disconnectAll();
		// onClear.disconnectAll();
		
		destroyAllContext();
		clear();
		if (!m_parent)
		{
			auto it = std::find(s_allRootLoggers.begin(), s_allRootLoggers.end(), this);
			if (it != s_allRootLoggers.end()) {
				s_allRootLoggers.erase(it);
			}
		}
		else
		{
			auto it = std::find(m_parent->m_childs.begin(), m_parent->m_childs.end(), this);
			if (it != m_parent->m_childs.end()) {
				m_parent->m_childs.erase(it);
			}
		}
		onDelete.emitSignal(*this);
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
		return s_allRootLoggers;
	}

	ContextLogger* ContextLogger::createContext(const std::string& name)
	{
		ContextLogger* context = new ContextLogger(name, this);
		m_childs.push_back(context);
#ifdef LOGGER_QT
		//emit onNewContext(this, context);
		emitRecursive_onNewContext(*this, *context);
		//if (m_subscribedUITree)
		//	m_subscribedUITree->onNewContext(*this, *context);
#endif
/*#ifdef LOGGER_QT
		if (m_treeWidget)
		{
			m_treeWidget->updateData(*this);
			//m_treeWidget->addChild(context->getTreeWidgetItem());
		}
#endif*/
		return context;
	}


	void ContextLogger::clear()
	{		
		m_messages.clear();
#ifdef LOGGER_QT
		emitRecursive_onClear(*this);
		//emit onClear();
		//if (m_subscribedUITree)
		//	m_subscribedUITree->onClear(*this);
#endif
		for (ContextLogger* c : m_childs)
			c->clear();
	}
	void ContextLogger::destroyAllContext()
	{
		for (size_t i=0; i< m_childs.size(); ++i)
		{
			m_childs[i]->destroyAllContext();
#ifdef LOGGER_QT
			emitRecursive_onDestroyContext(*m_childs[i]);
			//emit onDestroyContext(c);
			//if (m_subscribedUITree)
			//	m_subscribedUITree->onDestroyContext(*c);
#endif
			delete m_childs[i];
		}
			
		m_childs.clear();
	}
	void ContextLogger::destroyContext(ContextLogger* child)
	{
		auto it = std::find(m_childs.begin(), m_childs.end(), child);
		if (it == m_childs.end())
			return;
		m_childs.erase(it);
		child->destroyAllContext();
#ifdef LOGGER_QT
		emitRecursive_onDestroyContext(*child);
		//emit onDestroyContext(child);
		//if (m_subscribedUITree)
		//	m_subscribedUITree->onDestroyContext(*child);
#endif
		delete child;
	}


	void ContextLogger::logInternal(const Message& msg)
	{
		m_messages.push_back(msg);
#ifdef LOGGER_QT
		emitRecursive_onNewMessage(*this, msg);
		//emit onNewMessage(this, msg);
		//if(m_subscribedUITree)
		//   m_subscribedUITree->onNewMessage(*this, msg);
#endif
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
/*#ifdef LOGGER_QT
	void ContextLogger::subscribeToUI(QContextLoggerTree* tree) const
	{
		if (m_subscribedUITree == tree)
			return;
		subscribeToUI_internal(tree);
		if (m_subscribedUITree)
			m_subscribedUITree->onNewSubscribed(*m_rootParent);
		else
			m_subscribedUITree->onUnsubscribed(*m_rootParent);
		
	}
	void ContextLogger::unsubscribeFromUI() const
	{
		subscribeToUI(nullptr);
	}
	void ContextLogger::subscribeToUI_internal(QContextLoggerTree* tree) const
	{
		m_subscribedUITree = tree;
		for(size_t i=0; i<m_childs.size(); ++i)
			m_childs[i]->subscribeToUI_internal(tree);
	}
#endif*/
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


/*#ifdef LOGGER_QT
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
#endif*/

#ifdef LOGGER_QT
	void ContextLogger::updateUI()
	{

	}
#endif

	void ContextLogger::emitRecursive_onNewContext(const ContextLogger& parent, const ContextLogger& newContext)
	{
		if(onNewContext.getSlotCount())
			onNewContext.emitSignal(parent, newContext);
		if(m_parent)
			m_parent->emitRecursive_onNewContext(parent, newContext);
	}
	void ContextLogger::emitRecursive_onNewMessage(const ContextLogger& logger, const Message& newMessage)
	{
		if(onNewMessage.getSlotCount())
			onNewMessage.emitSignal(logger, newMessage);
		if(m_parent)
			m_parent->emitRecursive_onNewMessage(logger, newMessage);
	}
	void ContextLogger::emitRecursive_onDestroyContext(const ContextLogger& destroyedContext)
	{
		if(onDestroyContext.getSlotCount())
			onDestroyContext.emitSignal(destroyedContext);
		if(m_parent)
			m_parent->emitRecursive_onDestroyContext(destroyedContext);
	}
	void ContextLogger::emitRecursive_onClear(const ContextLogger& logger)
	{
		if(onClear.getSlotCount())
			onClear.emitSignal(logger);
		if (m_parent)
			m_parent->emitRecursive_onClear(logger);
	}
}