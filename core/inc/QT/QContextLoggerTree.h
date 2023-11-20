#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QTreeWidgetItem>
#include "ContextLogger.h"
#include <unordered_map>
#include <QTimer>


namespace Log
{
	class LOGGER_EXPORT QContextLoggerTree: public QWidget
	{
		Q_OBJECT
	public:
		enum class HeaderPos
		{
			contextName,
			timestamp,
			message,

			__count
		};
		QContextLoggerTree(QTreeWidget *parent = nullptr);
		~QContextLoggerTree();

		const QString &getHeaderName(HeaderPos pos) const;
		unsigned int getHeaderWidth(HeaderPos pos) const;

		
		void attachLogger(ContextLogger& logger);
		void detachLogger(ContextLogger& logger);
		
		

	private slots:
		void onUpdateTimer();

		void onNewSubscribed(const ContextLogger& logger);
		void onUnsubscribed(const ContextLogger& logger);

		void onNewContext(const ContextLogger& parent, const ContextLogger& newContext);
		void onNewMessage(const ContextLogger& logger, const Message& m);
		void onClear(const ContextLogger& logger);
		void onDestroyContext(const ContextLogger& loggerToDestroy);
		void onContextDelete(ContextLogger& loggerToDestroy);

	private:
		void updateMessageCount(const ContextLogger& logger, unsigned int &countOut);
		void updateMessageCountRecursive(const ContextLogger& logger, unsigned int &countOut);

		struct TreeData
		{
			QTreeWidgetItem* childRoot = nullptr;
			const ContextLogger* logger = nullptr;
			QTreeWidgetItem* thisMessagesRoot = nullptr;
			std::vector<QTreeWidgetItem*> msgItems;
		};


		QTreeWidget *m_treeWidget;

		

		std::unordered_map<const ContextLogger* , TreeData> m_msgItems;
		
		std::vector<const ContextLogger*> m_subscriber;
		

		QTimer m_updateTimer;
	};
}
#endif