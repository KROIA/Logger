#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include <vector>
#include <QTableView>
#include <QStyledItemDelegate>
#include <QTimer>
#include "Utilities/QLogMessageItemModel.h"
#include <QMutex>
#include <atomic>

namespace Log
{
    namespace UIWidgets
    {
        class LOGGER_API QConsoleWidget : public QTableView
        {
            Q_OBJECT
            public:
			QConsoleWidget(QWidget* parent = nullptr);
			~QConsoleWidget();

            void setDateTimeFormat(DateTime::Format format);
            DateTime::Format getDateTimeFormat() const;

            void setLevelVisibility(Level level, bool isVisible);
            void setContextVisibility(LoggerID loggerID, bool isVisible);
            void setDateTimeFilter(const DateTimeFilter& filter);
            void setTextFilter(const QString& text, bool useRegex);

            void onNewMessage(const Message& m);
            void onNewLogger(const LogObject::Info& info);
            void clear();

            // Number of rows that currently match the text filter (0 when no
            // filter is active).
            int getMatchCount() const;
            // Move selection to the next (or previous) filtered row.
            void findNext(bool forward);

            // Enable/disable the right-click row context menu.
            void setContextMenuEnabled(bool enabled);
            bool isContextMenuEnabled() const { return m_contextMenuEnabled; }

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list);

            // True while the view is anchored to the newest message (auto-scroll
            // follows incoming logs). Cleared when the user scrolls up, re-set
            // when the user scrolls back to the bottom.
            bool isStickToBottom() const { return m_stickToBottom; }
            // True while an in-cell text-selection editor is open. Following
            // pauses for its lifetime so the selection isn't torn away.
            bool hasActiveTextSelection() const { return m_persistentEditorIndex.isValid(); }
            // Close the in-cell editor and clear the current cell (bound to the
            // Escape key). Releases the follow-pause the selection caused.
            void clearTextSelection();

        signals:
            void filterChanged();
            // Emitted when the user requests context-level actions from the row menu.
            void requestSoloContext(Log::LoggerID id);
            void requestHideContext(Log::LoggerID id);
            void requestHideMessagesLike(const QString& text);
            // Emitted when the current row selection changes. Empty message = no selection.
            void selectionChangedMessage(const Log::Message& msg, bool hasSelection);

            void messageQueued(QPrivateSignal*);
        private slots:

            void onMessageQueued(QPrivateSignal*);

            void onAutoScrollTimerTimeout();
            void onScrollValueChanged(int value);

            // Get the resize event
            void resizeEvent(QResizeEvent* event) override;
            void showEvent(QShowEvent* event) override;
            // Double-click copies the clicked row's message text to the clipboard.
            void mouseDoubleClickEvent(QMouseEvent* event) override;
        protected:
            // Open a persistent read-only editor on the current cell so users can
            // drag-select text; keep it open across model updates.
            void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;
            void contextMenuEvent(QContextMenuEvent* event) override;
            void keyPressEvent(QKeyEvent* event) override;
        private:
            QPersistentModelIndex m_persistentEditorIndex;
            QString m_lastSearchText;
            bool m_contextMenuEnabled = true;

            QLogMessageItemModel* m_model;
            QLogMessageItemProxyModel * m_proxyModel;

            bool m_isAttaching = false;
            bool m_isDetaching = false;
            QTimer m_autoScrollTimer;
            bool m_stickToBottom = true;
            bool m_programmaticScroll = false;
            bool m_userScrollAction = false;

            void scrollToBottomGuarded();
            
            mutable QMutex m_mutex;
            std::vector<Message> m_messageQueue;
            std::atomic<bool> m_flushScheduled{ false };
        };
    }
}
#endif
