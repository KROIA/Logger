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
			DECLARE_SIGNAL_CONNECT_DISCONNECT(onContextDestroy, ContextLogger&);

			void clear() override;
			void destroyAllContext();
			void destroyContext(ContextLogger* child);

			void toStringVector(std::vector<std::string>& list) const;
			friend std::ostream& operator<<(std::ostream& os, const ContextLogger& msg);

			void getMessagesRecursive(std::vector<Message>& list) const;
			const std::vector<ContextLogger*>& getChilds() const;

		protected:

		private:
			void toStringVector(size_t depth, std::vector<std::string>& list) const;

			static std::vector<ContextLogger*>& getAllRootLoggers();
			
			std::vector<ContextLogger*> m_childs;
			ContextLogger* m_parent;
			ContextLogger* m_rootParent;

			Signal<ContextLogger&> onContextCreate;
			Signal<ContextLogger&> onContextDestroy;

			void emitRecursive_onContextCreate(ContextLogger& newContext);
			void emitRecursive_onContextDestroy(ContextLogger& destroyedContext);
		};
	}
}