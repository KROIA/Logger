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
#include <QColor>
#include <QRegularExpression>
#include <functional>


namespace Log
{
	namespace UIWidgets
	{
		class LOGGER_API QContextLoggerTreeWidget : public QWidget
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
			void onNewMessages(const std::vector<Message>& messages);
			void clearMessages();



			void setDateTimeFilter(const DateTimeFilter& filter);
			const DateTimeFilter& getDateTimeFilter() const;

			// Filter messages by substring or regex on the message text.
			// Empty text disables the filter.
			void setTextFilter(const QString& text, bool useRegex);
			bool matchesSearchText(const std::string& text) const;
			void setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType);
			void clearDateTimeFilter();
			const DateTime& getDateTimeFilterMin() const;
			const DateTime& getDateTimeFilterMax() const;
			DateTime::Range getDateTimeFilterRangeType() const;
			bool isDateTimeFilterActive() const;

			void setParent(LoggerID childID, LoggerID parentID);

			void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const;

			// True while the view is anchored to the newest message (auto-scroll
			// follows incoming logs). Cleared when the user scrolls up, re-set
			// when the user scrolls back to the bottom.
			bool isStickToBottom() const { return m_stickToBottom; }
			// True while an in-cell text-selection editor is open. Following
			// pauses for its lifetime so the selection isn't torn away.
			bool hasActiveTextSelection() const { return m_editorItem != nullptr; }
			// Close the in-cell editor and clear the current item (bound to the
			// Escape key). Releases the follow-pause the selection caused.
			void clearTextSelection();


		public slots:
			void setContextVisibility(LoggerID id, bool isVisible);
			bool getContextVisibility(LoggerID id) const;

			void setLevelVisibility(Level level, bool isVisible);
			bool getLevelVisibility(Level level) const;

		protected:
			bool eventFilter(QObject* obj, QEvent* ev) override;

		private slots:
			void onUpdateTimer();

		private:
			void scrollToBottomGuarded();
			//void addContextRecursive(Logger::ContextLogger& newContext);
			void updateMessageCount(unsigned int& countOut);
			void updateDateTimeFilter();

			class TreeData
			{
				friend class QContextLoggerTreeWidget;
				public:
					TreeData(QContextLoggerTreeWidget* root, const LogObject::Info& info);
					TreeData(QContextLoggerTreeWidget* root, TreeData *parent, const LogObject::Info& info);
					~TreeData();
					void updateDateTime();
					void onNewMessage(const Message& m);

					TreeData* createChild(const LogObject::Info& info);
					//void changeParent(LoggerID childID, TreeData* newParent);
					void setParent(TreeData* newParent);

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
					void applyTextFilter(const std::function<bool(const std::string&)>& matcher);

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
						dateTimeVisibility = 1,
						textVisibility = 2
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
				LogObject::Info m_info;
				QColor m_contextColor;
				QColor m_messageBackgroundColor;
			};
			
			QTreeWidget* m_treeWidget;
			bool m_levelVisibility[static_cast<unsigned int>(Level::__count)];

			std::unordered_map<LoggerID, TreeData*> m_msgItems;

		public:
			int getMatchCount() const;
			void findNext(bool forward);
			void setContextMenuEnabled(bool enabled) { m_contextMenuEnabled = enabled; }
			bool isContextMenuEnabled() const { return m_contextMenuEnabled; }
			void showRowContextMenu(QTreeWidgetItem* item, const QPoint& globalPos);
		signals:
			void requestSoloContext(Log::LoggerID id);
			void requestHideContext(Log::LoggerID id);
			void requestHideMessagesLike(const QString& text);
			void selectionChangedMessage(const Log::Message& msg, bool hasSelection);
		private:
			bool m_contextMenuEnabled = true;
			std::vector<QTreeWidgetItem*> collectVisibleMessageItems() const;

			// In-cell text-selection editor bookkeeping (opened on click only).
			QTreeWidgetItem* m_editorItem = nullptr;
			int m_editorColumn = -1;
			// Stick-to-bottom mechanic — same design as QConsoleWidget: the flag
			// is driven only by genuine user scroll actions; programmatic scrolls
			// are guarded so they can't feed back into it.
			bool m_stickToBottom = true;
			bool m_programmaticScroll = false;
			bool m_userScrollAction = false;

			QTimer m_updateTimer;
			DateTime::Format m_timeFormat;
			DateTimeFilter m_dateTimeFilter;
			QString m_searchText;
			bool m_searchUseRegex = false;
			bool m_searchNegate = false;
			QRegularExpression m_searchRegex;
			bool m_messageCountDirty = false;
		};
	}
}
#endif
