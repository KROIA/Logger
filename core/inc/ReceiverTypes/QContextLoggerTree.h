#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QTreeWidgetItem>
#include "LoggerTypes/ContextLogger.h"
#include <unordered_map>
#include <QTimer>
#include "ContextReceiver.h"


namespace Log
{
	namespace Receiver
	{
		class LOGGER_EXPORT QContextLoggerTree : public QWidget, public ContextReceiver
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
			QContextLoggerTree(QTreeWidget* parent = nullptr);
			~QContextLoggerTree();

			const QString& getHeaderName(HeaderPos pos) const;
			unsigned int getHeaderWidth(HeaderPos pos) const;



		private slots:
			void onUpdateTimer();

			void onNewSubscribed(Logger::AbstractLogger& logger) override;
			void onUnsubscribed(Logger::AbstractLogger& logger) override;

			void onContextCreate(Logger::ContextLogger& newContext) override;
			void onNewMessage(const Message& m) override;
			void onClear(Logger::AbstractLogger& logger) override;
			void onContextDestroy(Logger::ContextLogger& loggerToDestroy) override;
			void onDelete(Logger::AbstractLogger& loggerToDestroy) override;

		private:
			void updateMessageCount(const Logger::ContextLogger& logger, unsigned int& countOut);
			void updateMessageCountRecursive(const Logger::ContextLogger& logger, unsigned int& countOut);

			class TreeData : public ContextReceiver
			{
				public:
				QTreeWidgetItem* childRoot = nullptr;
				const Logger::ContextLogger* logger = nullptr;
				QTreeWidgetItem* thisMessagesRoot = nullptr;
				std::vector<QTreeWidgetItem*> msgItems;

				void onNewSubscribed(Logger::AbstractLogger& logger) override;
				void onUnsubscribed(Logger::AbstractLogger& logger) override;
				void onNewMessage(const Message& m) override;
				void onClear(Logger::AbstractLogger& logger) override;
				void onDelete(Logger::AbstractLogger& logger) override;
				void onContextCreate(Logger::ContextLogger& logger) override;
				void onContextDestroy(Logger::ContextLogger& logger) override;
			};


			QTreeWidget* m_treeWidget;



			std::unordered_map<const Logger::AbstractLogger*, TreeData*> m_msgItems;

			QTimer m_updateTimer;
		};
	}
}
#endif