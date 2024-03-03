#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QTreeWidgetItem>
#include "LoggerTypes/ContextLogger.h"
#include <unordered_map>
#include <QTimer>
#include "ReceiverTypes/ContextReceiver.h"
#include "LogLevel.h"


namespace Log
{
	namespace Receiver
	{
		class LOGGER_EXPORT QContextLoggerTree : public QWidget//, public ContextReceiver
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

			void addContext(Logger::AbstractLogger& newContext);
			void removeContext(Logger::AbstractLogger::LoggerID id);
			//void loggerDeleted(Logger::AbstractLogger::LoggerID id);
			void onNewMessage(const Message& m);
			void clearMessages();
		public slots:
			void setContextVisibility(Logger::AbstractLogger::LoggerID& id, bool isVisible);
			bool getContextVisibility(Logger::AbstractLogger::LoggerID& id) const;

			void setLevelVisibility(Level level, bool isVisible);
			bool getLevelVisibility(Level level) const;

		private slots:
			void onUpdateTimer();		

		private:
			void addContextRecursive(Logger::ContextLogger& newContext);
			void updateMessageCount(unsigned int& countOut);

			class TreeData
			{
				public:
					TreeData(QContextLoggerTree* root, const Logger::AbstractLogger&logger);
					TreeData(QContextLoggerTree* root, TreeData *parent, const Logger::AbstractLogger& logger);
					~TreeData();
					void updateDateTime();
					void onNewMessage(const Message& m);

					TreeData* createChild(Logger::AbstractLogger& newContext);

					void getLoggerIDsRecursive(std::vector<Logger::AbstractLogger::LoggerID> &list) const;
					void getChildLoggerIDsRecursive(std::vector<Logger::AbstractLogger::LoggerID> &list) const;
			
					void setContextVisibility(bool isVisible);
					bool getContextVisibility() const;

					void setLevelVisibility(Level level, bool isVisible);
					void updateMessageCount(unsigned int& countOut);

					void clearMessages();
					void clearMessagesRecursive();

					bool getLoggerIsAlive() const;

					TreeData *getParent() const;
			private:
				void setupChildRoot();
				void setupMessageRoot();
				
				QTreeWidgetItem* childRoot = nullptr;
				QTreeWidgetItem* thisMessagesRoot = nullptr;

				struct MessageData
				{
					Message::SnapshotData snapshot;
					QTreeWidgetItem* item = nullptr;
				};
				std::shared_ptr<Logger::AbstractLogger::LoggerMetaInfo> loggerMetaInfo;
				std::vector<MessageData> msgItems;
				std::vector<TreeData*> children;
				TreeData *parent = nullptr;
				QContextLoggerTree *root = nullptr;
			};
			


			QTreeWidget* m_treeWidget;
			bool m_levelVisibility[static_cast<unsigned int>(Level::__count)];


			std::unordered_map<Logger::AbstractLogger::LoggerID, TreeData*> m_msgItems;

			QTimer m_updateTimer;
			DateTime::Format m_timeFormat;
		};
	}
}
#endif