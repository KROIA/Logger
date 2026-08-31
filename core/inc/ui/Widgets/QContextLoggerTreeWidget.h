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

			// Called when a known logger's Info changes at runtime (policy,
			// display preference, color, name, parent, ...). Refreshes
			// m_knownInfos (and the materialized TreeData's cached Info, if
			// any) then reconciles this logger's tree placement (materialize /
			// demote / re-point / redirect) accordingly.
			void onLoggerInfoChanged(const LogObject::Info& info);

			// When true (default), a logger's contextDisplayPolicy ==
			// FlattenSuggested is honored (its messages fold into the nearest
			// materialized ancestor instead of getting their own node). When
			// false, every known logger is (re)materialized with its own
			// context. Reconciles every known logger on a real change.
			void setRespectFlattenSuggestions(bool respect);

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
					// newParent == nullptr promotes this to a top-level item.
					void setParent(TreeData* newParent);

					// Refreshes cached Info/colors (name, color, creation time) —
					// Info can change at runtime via onLoggerInfoChanged, and prior
					// to this TreeData cached it once at construction and never
					// refreshed it.
					void updateInfo(const LogObject::Info& info);
					// Re-parents (into destination) every message this TreeData is
					// currently holding whose original logger id is
					// sourceLoggerID. Used both to pull back previously-redirected
					// messages when a logger newly materializes, and to push this
					// node's own (or forwarded) messages onward when it
					// demotes/re-redirects.
					void migrateMessagesFor(LoggerID sourceLoggerID, TreeData* destination);

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

			// Every Info ever seen (via addContext/onLoggerInfoChanged), including
			// loggers that never materialize a TreeData (Invisible, or honored
			// FlattenSuggested), so ancestor walks work without depending on
			// LogManager — this widget is deliberately decoupled from it.
			std::unordered_map<LoggerID, LogObject::Info> m_knownInfos;
			// Flattened-logger-id -> materialized-ancestor-id it forwards its
			// messages into (no TreeData of its own).
			std::unordered_map<LoggerID, LoggerID> m_redirectTarget;
			bool m_respectFlattenSuggestions = true;

			// Walks the parentId chain (via m_knownInfos) starting at
			// startParentId until it finds an id present in m_msgItems (i.e. it
			// has a materialized TreeData), or returns 0. Capped to tolerate a
			// cycle defensively.
			LoggerID resolveTreeParent(LoggerID startParentId) const;
			// Called whenever a known logger's policy/display-preference/parent
			// changes at runtime; materializes/demotes/re-points/redirects it
			// (and whatever depended on its old placement) as needed.
			void reconcileLoggerPlacement(LoggerID id);

		public:
			int getMatchCount() const;
			void findNext(bool forward);
			void setContextMenuEnabled(bool enabled) { m_contextMenuEnabled = enabled; }
			bool isContextMenuEnabled() const { return m_contextMenuEnabled; }
			void showRowContextMenu(QTreeWidgetItem* item, const QPoint& globalPos);

			// Loggers with ReceiverVisibilityPolicy::ManualAdd that are
			// currently materialized but hidden (context visibility off).
			std::vector<LoggerID> getManuallyHiddenLoggerIds() const;
		signals:
			void requestSoloContext(Log::LoggerID id);
			void requestHideContext(Log::LoggerID id);
			void requestHideMessagesLike(const QString& text);
			// Emitted when the user picks a hidden logger from the "Show hidden
			// logger" context-menu submenu.
			void requestShowLogger(Log::LoggerID id);
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
