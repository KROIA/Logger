#pragma once

#include "AbstractLogger.h"
#include <vector>
#include "Signal.h"


namespace Log
{

	class LOGGER_EXPORT ContextLogger : public AbstractLogger
	{
		ContextLogger(const std::string& name, ContextLogger* parent);
	public:
		ContextLogger(const std::string& name = "");
		ContextLogger(const ContextLogger& other);

		~ContextLogger();

		ContextLogger* getParent() const;
		ContextLogger* getRootParent() const;
		static const std::vector<ContextLogger*>& getAllLogger();

		ContextLogger* createContext(const std::string& name);


		DECLARE_SIGNAL_CONNECT_DISCONNECT(onNewContext, const ContextLogger&, const ContextLogger&);
		DECLARE_SIGNAL_CONNECT_DISCONNECT(onNewMessage, const ContextLogger&, const Message&);
		DECLARE_SIGNAL_CONNECT_DISCONNECT(onDestroyContext, const ContextLogger&);
		DECLARE_SIGNAL_CONNECT_DISCONNECT(onClear, const ContextLogger&);
		DECLARE_SIGNAL_CONNECT_DISCONNECT(onDelete, ContextLogger&);

		void clear();
		void destroyAllContext();
		void destroyContext(ContextLogger* child);

		void toStringVector(std::vector<std::string>& list) const;
		friend std::ostream& operator<<(std::ostream& os, const ContextLogger& msg);

		const DateTime& getDateTime() const;
		const std::vector<Message>& getMessages() const;
		const std::vector<ContextLogger*>& getChilds() const;

	private:
		void toStringVector(size_t depth, std::vector<std::string>& list) const;
		void logInternal(const Message& msg) override;

		DateTime m_creationsTime;
		std::vector<Message> m_messages;
		std::vector<ContextLogger*> m_childs;
		ContextLogger* m_parent;
		ContextLogger* m_rootParent;

		Signal<const ContextLogger&, const ContextLogger&> onNewContext;
		Signal<const ContextLogger&, const Message&> onNewMessage;
		Signal<const ContextLogger&> onDestroyContext;
		Signal<const ContextLogger&> onClear;
		Signal<ContextLogger&> onDelete;

		void emitRecursive_onNewContext(const ContextLogger& parent, const ContextLogger& newContext);
		void emitRecursive_onNewMessage(const ContextLogger& logger, const Message& newMessage);
		void emitRecursive_onDestroyContext(const ContextLogger& destroyedContext);
		void emitRecursive_onClear(const ContextLogger& logger);
		
		static std::vector<ContextLogger*> s_allRootLoggers;
	};
}