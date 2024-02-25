#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QTreeWidgetItem>
#include "LoggerTypes/ContextLogger.h"
#include <unordered_map>
#include <QTimer>
#include "ContextReceiver.h"
#include "LogLevel.h"


namespace Log
{
	namespace Receiver
	{
		class LOGGER_EXPORT QContextLoggerTree : public QWidget, public ContextReceiver
		{
			Q_OBJECT
			friend class TreeData;
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

			void setDateTimeFormat(DateTime::Format format);
			DateTime::Format getDateTimeFormat() const;

		public slots:
			void setContextVisibility(Logger::ContextLogger& logger, bool isVisible);
			bool getContextVisibility(Logger::ContextLogger& logger) const;

			void setLevelVisibility(Level level, bool isVisible);
			bool getLevelVisibility(Level level) const;

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
			void addContextRecursive(Logger::ContextLogger& newContext);
			void updateMessageCount(const Logger::ContextLogger& logger, unsigned int& countOut);
			void updateMessageCountRecursive(const Logger::ContextLogger& logger, unsigned int& countOut);

			class TreeData : public ContextReceiver
			{
				public:
				QTreeWidgetItem* childRoot = nullptr;
				const Logger::ContextLogger* logger = nullptr;
				QTreeWidgetItem* thisMessagesRoot = nullptr;
				std::vector<QTreeWidgetItem*> msgItems;

				void updateDateTime();

				void onNewSubscribed(Logger::AbstractLogger& logger) override;
				void onUnsubscribed(Logger::AbstractLogger& logger) override;
				void onNewMessage(const Message& m) override;
				void onClear(Logger::AbstractLogger& logger) override;
				void onDelete(Logger::AbstractLogger& logger) override;
				void onContextCreate(Logger::ContextLogger& logger) override;
				void onContextDestroy(Logger::ContextLogger& logger) override;

				QContextLoggerTree *parent = nullptr;
			};
			


			QTreeWidget* m_treeWidget;
			bool m_levelVisibility[static_cast<unsigned int>(Level::__count)];


			std::unordered_map<const Logger::AbstractLogger*, TreeData*> m_msgItems;

			QTimer m_updateTimer;
			DateTime::Format m_timeFormat;
		};
	}
}
#endif