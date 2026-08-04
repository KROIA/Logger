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

            void onNewMessage(const Message& m);
            void onNewLogger(const LogObject::Info& info);
            void clear();

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list);

        signals:
            void messageQueued(QPrivateSignal*);
        private slots:

            void onMessageQueued(QPrivateSignal*);

            void onAutoScrollTimerTimeout();
            void onVertialSliderMoved(int value);

            // Get the resize event
            void resizeEvent(QResizeEvent* event) override;
            // Double-click copies the clicked row's message text to the clipboard.
            void mouseDoubleClickEvent(QMouseEvent* event) override;
        protected:
            // Open a persistent read-only editor on the current cell so users can
            // drag-select text; keep it open across model updates.
            void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;
        private:
            QPersistentModelIndex m_persistentEditorIndex;

            QLogMessageItemModel* m_model;
            QLogMessageItemProxyModel * m_proxyModel;

            bool m_isAttaching = false;
            bool m_isDetaching = false;
            QTimer m_autoScrollTimer;
            
            mutable QMutex m_mutex;
            std::vector<Message> m_messageQueue;
            std::atomic<bool> m_flushScheduled{ false };
        };
    }
}
#endif
