#pragma once
#include "Logger_base.h"
#include "LogObject.h"

#ifdef QT_WIDGETS_LIB
#include <QTreeWidgetItem>
#include "Utilities/DateTime.h"
#include <unordered_map>
#include <QTimer>
#include "LogLevel.h"
#include <QDebug>


namespace Log
{
	namespace UIWidgets
	{
		class LOGGER_EXPORT QContextLoggerTreeWidget : public QWidget
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
			QContextLoggerTreeWidget(QTreeWidget* parent = nullptr);
			~QContextLoggerTreeWidget();


			const QString& getHeaderName(HeaderPos pos) const;
			unsigned int getHeaderWidth(HeaderPos pos) const;

			void setDateTimeFormat(DateTime::Format format);
			DateTime::Format getDateTimeFormat() const;

			void addContext(const LogObject::Info &newContext);
			//void removeContext(LoggerID id);
			void onNewMessage(const Message& m);
			void clearMessages();



			void setDateTimeFilter(const DateTimeFilter& filter);
			const DateTimeFilter& getDateTimeFilter() const;
			void setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType);
			void clearDateTimeFilter();
			const DateTime& getDateTimeFilterMin() const;
			const DateTime& getDateTimeFilterMax() const;
			DateTime::Range getDateTimeFilterRangeType() const;
			bool isDateTimeFilterActive() const;

			void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const;


		public slots:
			void setContextVisibility(LoggerID id, bool isVisible);
			bool getContextVisibility(LoggerID id) const;

			void setLevelVisibility(Level level, bool isVisible);
			bool getLevelVisibility(Level level) const;

		private slots:
			void onUpdateTimer();		

		private:
			//void addContextRecursive(Logger::ContextLogger& newContext);
			void updateMessageCount(unsigned int& countOut);
			void updateDateTimeFilter();

			class TreeData
			{
				public:
					TreeData(QContextLoggerTreeWidget* root, LoggerID loggerID);
					TreeData(QContextLoggerTreeWidget* root, TreeData *parent, LoggerID loggerID);
					~TreeData();
					void updateDateTime();
					void onNewMessage(const Message& m);

					TreeData* createChild(LoggerID loggerID);

					void getLoggerIDsRecursive(std::vector<LoggerID> &list) const;
					void getChildLoggerIDsRecursive(std::vector<LoggerID> &list) const;
			
					void setContextVisibility(bool isVisible);
					bool getContextVisibility() const;

					void setLevelVisibility(Level level, bool isVisible);
					void updateMessageCount(unsigned int& countOut);

					void clearMessages();
					void clearMessagesRecursive();

					//bool getLoggerIsAlive() const;

					TreeData *getParent() const;

					void updateDateTimeFilter(const DateTimeFilter &filter);

					void saveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const;
			
			private:
				void setupChildRoot();
				void setupMessageRoot();
				
				QTreeWidgetItem* childRoot = nullptr;
				QTreeWidgetItem* thisMessagesRoot = nullptr;

				struct MessageData
				{
					Message msg;
					QTreeWidgetItem* item = nullptr;

					enum VisibilityBitMask
					{
						levelVisibility = 0,
						dateTimeVisibility = 1
					};
					int hideFilter = 0;
					void setVisibilityFilter(VisibilityBitMask mask, bool isVisible)
					{
						if (isVisible)
						{
							hideFilter &= ~(1 << mask);
						}
						else
						{
							hideFilter |= 1 << mask;
						}
						item->setHidden(hideFilter != 0);
					}
					bool isVisible() const
					{
						return hideFilter == 0;
					}
				};
				//std::shared_ptr<const Logger::AbstractLogger::MetaInfo> MetaInfo;
				std::vector<MessageData> msgItems;
				std::vector<TreeData*> children;
				TreeData *parent = nullptr;
				QContextLoggerTreeWidget *root = nullptr;
				LoggerID loggerID;
			};
			
			QTreeWidget* m_treeWidget;
			bool m_levelVisibility[static_cast<unsigned int>(Level::__count)];

			std::unordered_map<LoggerID, TreeData*> m_msgItems;

			QTimer m_updateTimer;
			DateTime::Format m_timeFormat;
			DateTimeFilter m_dateTimeFilter;
		};
	}
}
#endif