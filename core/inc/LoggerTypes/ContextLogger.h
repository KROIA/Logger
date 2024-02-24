#pragma once

#include "AbstractLogger.h"
#include <vector>
#include "Utilities/Signal.h"


namespace Log
{
	namespace Logger
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


			DECLARE_SIGNAL_CONNECT_DISCONNECT(onContextCreate, ContextLogger&);
			//DECLARE_SIGNAL_CONNECT_DISCONNECT(onNewMessage, const Message&);
			DECLARE_SIGNAL_CONNECT_DISCONNECT(onContextDestroy, ContextLogger&);
			//DECLARE_SIGNAL_CONNECT_DISCONNECT(onClear, const ContextLogger&);
			//DECLARE_SIGNAL_CONNECT_DISCONNECT(onDelete, ContextLogger&);

			void clear() override;
			void destroyAllContext();
			void destroyContext(ContextLogger* child);

			void toStringVector(std::vector<std::string>& list) const;
			friend std::ostream& operator<<(std::ostream& os, const ContextLogger& msg);

			//const DateTime& getCreationDateTime() const;
			//const std::vector<Message>& getMessages() const;
			void getMessagesRecursive(std::vector<Message>& list) const;
			const std::vector<ContextLogger*>& getChilds() const;

		protected:
			void logInternal(const Message& msg) override;
		private:
			void toStringVector(size_t depth, std::vector<std::string>& list) const;
			//void logInternal(const Message& msg) override;

			static std::vector<ContextLogger*>& getAllRootLoggers();

			//DateTime m_creationsTime;
			
			std::vector<ContextLogger*> m_childs;
			ContextLogger* m_parent;
			ContextLogger* m_rootParent;

			Signal<ContextLogger&> onContextCreate;
			//Signal<const Message&> onNewMessage;
			Signal<ContextLogger&> onContextDestroy;
			//Signal<const ContextLogger&> onClear;
			//Signal<ContextLogger&> onDelete;

			void emitRecursive_onContextCreate(ContextLogger& newContext);
			//void emitRecursive_onNewMessage(const Message& newMessage);
			void emitRecursive_onContextDestroy(ContextLogger& destroyedContext);
			//void emitRecursive_onClear(const ContextLogger& logger);
		};
	}
}