#pragma once

#include "AbstractLogger.h"
#include <vector>
#include <sstream>
#include "Signal.h"

#ifdef LOGGER_QT

#endif


namespace Log
{

	class LOGGER_EXPORT ContextLogger :
#ifdef LOGGER_QT
	//	public QObject,
#endif
		public AbstractLogger
	{
#ifdef LOGGER_QT
		//Q_OBJECT
			//friend QContextLoggerTree;
#endif

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
#ifdef LOGGER_QT
/*	signals:
		void onNewContext(const ContextLogger* parent, const ContextLogger* newContext);
		void onNewMessage(const ContextLogger* logger, const Message& newMessage);
		void onClear();
		void onDestroyContext(const ContextLogger* destroyedContext);
*/

		//	void subscribeToUI(QContextLoggerTree* tree) const;
		//	void unsubscribeFromUI() const;
#endif
// #ifdef LOGGER_QT
// 		QContextLoggerTreeWidgetItem* getTreeWidgetItem();
// #endif
	private:
		void toStringVector(size_t depth, std::vector<std::string>& list) const;
		void logInternal(const Message& msg) override;

#ifdef LOGGER_QT
		//void subscribeToUI_internal(QContextLoggerTree* tree) const;
		void updateUI();
#endif
		// #ifdef LOGGER_QT
		// 		void updateTreeWidgetItem();
		// #endif

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


		//mutable std::vector<QContextLoggerTree*> m_subscribedUITree;
#ifdef LOGGER_QT
	//	mutable QContextLoggerTree* m_subscribedUITree;
#endif
//#ifdef LOGGER_QT
//		QContextLoggerTreeWidgetItem* m_treeWidget;
//#endif
		
		static std::vector<ContextLogger*> s_allRootLoggers;
	};
}