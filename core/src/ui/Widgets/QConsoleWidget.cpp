#include "ui/Widgets/QConsoleWidget.h"
#include "ReadOnlyCellEditorDelegate.h"

#ifdef QT_WIDGETS_LIB
#include <QHeaderView>
#include <QScrollBar>
#include <QApplication>
#include <QThread>
#include <QClipboard>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <algorithm>
#include <functional>




namespace Log
{
    namespace UIWidgets
    {


        QConsoleWidget::QConsoleWidget(QWidget* parent)
            : QTableView(parent)
        {
            m_model = new QLogMessageItemModel(this);
            m_proxyModel = new QLogMessageItemProxyModel(this);
            m_proxyModel->setSourceModel(m_model);

            setModel(m_proxyModel);
            setWordWrap(false);
            setShowGrid(false);

            QHeaderView* header = horizontalHeader();
            header->setStretchLastSection(true);
            header->setSectionResizeMode(QLogMessageItemModel::Column::TimeColumn, QHeaderView::Interactive);
            header->setSectionResizeMode(QLogMessageItemModel::Column::LevelColumn, QHeaderView::Interactive);
            header->setSectionResizeMode(QLogMessageItemModel::Column::ContextColumn, QHeaderView::Interactive);
            header->setSectionResizeMode(QLogMessageItemModel::Column::MessageColumn, QHeaderView::Stretch);

            setColumnWidth(QLogMessageItemModel::Column::TimeColumn, 150);
            setColumnWidth(QLogMessageItemModel::Column::LevelColumn, 50);
            setColumnWidth(QLogMessageItemModel::Column::ContextColumn, 100);

            QHeaderView* vHeader = verticalHeader();
            vHeader->setDefaultSectionSize(10);
            vHeader->setMinimumSectionSize(15);

            // Simple sticky-to-bottom: flip TRUE when the scrollbar reaches
            // the max, FALSE when the user drags it above the max.
            // Neither Qt signal alone can drive the flag safely:
            //  - actionTriggered fires only for user actions, but BEFORE the
            //    value is applied, so sampling it later races against message
            //    batches growing the range (re-engaging stickiness at the
            //    bottom becomes impossible under load).
            //  - valueChanged samples value+max at the same instant, but also
            //    fires for Qt-internal scrolls (layout restore after a tab
            //    switch, geometry updates), which must never flip the flag.
            // So: actionTriggered arms m_userScrollAction, and the directly
            // following valueChanged (same call stack) evaluates the flag.
            // Value changes that were not armed by a user action are ignored.
            connect(verticalScrollBar(), &QAbstractSlider::actionTriggered,
                    this, [this](int) { m_userScrollAction = true; });
            connect(verticalScrollBar(), &QAbstractSlider::valueChanged,
                    this, &QConsoleWidget::onScrollValueChanged);
            // The scrollbar range updates lazily: while the view is hidden or
            // mid-relayout (e.g. returning from another tab), maximum() is
            // stale and scrollToBottom() lands short. rangeChanged fires at
            // the exact moment Qt finalizes the real range, so re-clamp there.
            connect(verticalScrollBar(), &QScrollBar::rangeChanged,
                    this, [this](int, int max) {
                        if (m_stickToBottom && !m_persistentEditorIndex.isValid())
                        {
                            m_programmaticScroll = true;
                            verticalScrollBar()->setValue(max);
                            m_programmaticScroll = false;
                        }
                    });
            m_autoScrollTimer.setInterval(100);
            connect(&m_autoScrollTimer, &QTimer::timeout, this, &QConsoleWidget::onAutoScrollTimerTimeout);
            m_autoScrollTimer.start();

            connect(this, &QConsoleWidget::messageQueued, this, &QConsoleWidget::onMessageQueued, Qt::QueuedConnection);

            // Coalescing gate. Without it the queue flushes once per event-loop
            // turn, which under a live producer is once per message — the model
            // insert, the proxy re-filter and the scroll then run thousands of
            // times for updates nobody can see.
            m_flushTimer.setSingleShot(true);
            m_flushTimer.setInterval(100);
            connect(&m_flushTimer, &QTimer::timeout, this, &QConsoleWidget::onFlushTimeout);

            // In-cell text selection: single click on a cell opens a persistent
            // read-only line editor over it, so users can drag-select individual
            // text. The editor persists across model updates (new messages don't
            // clear the user's selection).
            setItemDelegate(new Internal::ReadOnlyCellEditorDelegate(
                QLogMessageItemModel::Column::MessageColumn,
                [this]() { clearTextSelection(); },
                this));
            setEditTriggers(QAbstractItemView::NoEditTriggers); // we open editors manually on cell click
            setSelectionMode(QAbstractItemView::NoSelection);   // no row highlight — user selects text, not rows
            setTextElideMode(Qt::ElideNone);

            // Open the persistent editor only on a genuine cell CLICK.
            // currentChanged is NOT a safe trigger: Qt also moves the current
            // index on focus restore (e.g. returning from another tab), and an
            // editor opened by that is invisible to the user (it looks exactly
            // like the cell) while silently blocking stick-to-bottom.
            connect(this, &QAbstractItemView::pressed,
                    this, [this](const QModelIndex& idx) {
                        if (!idx.isValid())
                            return;
                        if (!(QGuiApplication::mouseButtons() & Qt::LeftButton))
                            return;
                        if (m_persistentEditorIndex.isValid())
                        {
                            if (QModelIndex(m_persistentEditorIndex) == idx)
                                return; // already open on this cell
                            closePersistentEditor(QModelIndex(m_persistentEditorIndex));
                        }
                        openPersistentEditor(idx);
                        m_persistentEditorIndex = QPersistentModelIndex(idx);
                    });
        }
        QConsoleWidget::~QConsoleWidget()
        {

        }

        void QConsoleWidget::setDateTimeFormat(DateTime::Format format)
        {
            m_model->setDateTimeFormat(format);
        }
        DateTime::Format QConsoleWidget::getDateTimeFormat() const
        {
            return m_model->getDateTimeFormat();
        }

        void QConsoleWidget::setLevelVisibility(Level level, bool isVisible)
        {
            m_proxyModel->setLevelVisibility(level, isVisible);
        }
        void QConsoleWidget::setContextVisibility(LoggerID loggerID, bool isVisible)
        {
            m_proxyModel->setContextVisibility(loggerID, isVisible);
        }
        void QConsoleWidget::setDateTimeFilter(const DateTimeFilter& filter)
        {
            m_proxyModel->setDateTimeFilter(filter);
        }
        void QConsoleWidget::setTextFilter(const QString& text, bool useRegex)
        {
            m_lastSearchText = text;
            m_proxyModel->setTextFilter(text, useRegex);
            emit filterChanged();
        }

        int QConsoleWidget::getMatchCount() const
        {
            if (m_lastSearchText.isEmpty())
                return 0;
            return m_proxyModel->rowCount();
        }
        void QConsoleWidget::findNext(bool forward)
        {
            const int n = m_proxyModel->rowCount();
            if (n == 0)
                return;
            int startRow = -1;
            const QModelIndex cur = currentIndex();
            if (cur.isValid())
                startRow = cur.row();
            int nextRow;
            if (forward)
                nextRow = (startRow + 1) % n;
            else
                nextRow = (startRow <= 0) ? (n - 1) : (startRow - 1);
            const QModelIndex target = m_proxyModel->index(nextRow, cur.isValid() ? cur.column() : 0);
            setCurrentIndex(target);
            scrollTo(target, QAbstractItemView::PositionAtCenter);
        }
        void QConsoleWidget::setContextMenuEnabled(bool enabled)
        {
            m_contextMenuEnabled = enabled;
        }

        void QConsoleWidget::onNewLogger(const LogObject::Info& info)
        {
            const ReceiverVisibilityPolicy previousPolicy = m_model->setLoggerInfo(info);
            if (info.visibilityPolicy == ReceiverVisibilityPolicy::Invisible)
            {
                // Always force-hidden, regardless of the user's manual toggle.
                setContextVisibility(info.id, false);
            }
            else if (previousPolicy != info.visibilityPolicy)
            {
                // A detected policy transition (e.g. ManualAdd <-> AutoVisible,
                // or coming back from Invisible) resets the visibility to the
                // new policy's default; otherwise leave the user's existing
                // manual toggle alone.
                setContextVisibility(info.id, info.visibilityPolicy != ReceiverVisibilityPolicy::ManualAdd);
            }
        }
        void QConsoleWidget::setRefreshInterval(int intervalMs)
        {
            m_flushTimer.setInterval(std::max(1, intervalMs));
        }
        int QConsoleWidget::getRefreshInterval() const
        {
            return m_flushTimer.interval();
        }

        void QConsoleWidget::clear()
        {
            {
                QMutexLocker locker(&m_mutex);
                m_messageQueue.clear();
                m_flushScheduled.store(false);
            }
            m_flushTimer.stop();
            m_model->clear();
            m_model->clearLoggerCache();
        }
        void QConsoleWidget::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list)
        {
            if (QApplication::instance() && QApplication::instance()->thread() != QThread::currentThread())
            {  }
            else
                // Bypass the coalescing gate: a save must see every message
                // that has already been queued, not just the flushed ones.
                flushQueue();

            int count = m_model->rowCount();
            list.reserve(count);
            for (int i = 0; i < count; ++i)
            {
                if (m_proxyModel->filterAcceptsRow(i))
                {
                    const Message& data = m_model->getElement(i);
                    list[data.getLoggerID()].push_back(data);
                }
            }
        }

        void QConsoleWidget::onAutoScrollTimerTimeout()
        {
            // Don't scroll while the user has an active in-cell text selection —
            // scrolling would tear it out from under them.
            if (m_persistentEditorIndex.isValid())
                return;
            // Only reconcile to bottom when the user hasn't scrolled up. Without
            // this guard the timer would fight the user's scroll every 100ms.
            if (!m_stickToBottom)
                return;
            scrollToBottomGuarded();
        }

        void QConsoleWidget::onScrollValueChanged(int value)
        {
            // Consume the "armed by a user scroll action" marker regardless of
            // the early-outs below so a stale marker can't linger.
            const bool userAction = m_userScrollAction;
            m_userScrollAction = false;
            if (m_programmaticScroll)
                return;
            // Only value changes directly caused by a user scroll action may
            // drive the flag; Qt-internal scrolls (layout restore after a tab
            // switch, geometry updates) are ignored.
            if (!userAction)
                return;
            // Ignore anything sampled while hidden or mid tab-switch: the
            // scrollbar geometry is unreliable there and must never unstick us.
            if (!isVisible())
                return;
            const int max = verticalScrollBar()->maximum();
            m_stickToBottom = (max - value <= 1);
        }

        void QConsoleWidget::scrollToBottomGuarded()
        {
            m_programmaticScroll = true;
            scrollToBottom();
            m_programmaticScroll = false;
        }

        void QConsoleWidget::resizeEvent(QResizeEvent* event)
        {
            QTableView::resizeEvent(event);
            // Resize the row height to fit the text
            this->resizeRowsToContents();
            if (m_stickToBottom)
                scrollToBottomGuarded();
        }

        void QConsoleWidget::showEvent(QShowEvent* event)
        {
            QTableView::showEvent(event);
            // Coming back from another tab: Qt has just laid the widget out
            // again. Any scrollToBottom() we did while hidden won't have taken
            // effect against the current geometry. Re-anchor once the pending
            // layout has settled.
            if (m_stickToBottom)
            {
                QMetaObject::invokeMethod(this,
                    [this]() { if (m_stickToBottom) scrollToBottomGuarded(); },
                    Qt::QueuedConnection);
            }
        }

        void QConsoleWidget::currentChanged(const QModelIndex& current, const QModelIndex& previous)
        {
            QTableView::currentChanged(current, previous);
            // Close the editor of the cell we're leaving. The new editor (if
            // any) is opened by the click handler, not here — currentChanged
            // also fires for focus-driven index changes that must not spawn one.
            if (m_persistentEditorIndex.isValid() && QModelIndex(m_persistentEditorIndex) != current)
            {
                closePersistentEditor(QModelIndex(m_persistentEditorIndex));
                m_persistentEditorIndex = QPersistentModelIndex();
            }
            if (current.isValid())
            {
                const QModelIndex src = m_proxyModel->mapToSource(current);
                if (src.row() >= 0 && src.row() < m_model->rowCount())
                    emit selectionChangedMessage(m_model->getElement(src.row()), true);
            }
            else
            {
                emit selectionChangedMessage(Message(), false);
            }
        }

        void QConsoleWidget::clearTextSelection()
        {
            if (m_persistentEditorIndex.isValid())
            {
                closePersistentEditor(QModelIndex(m_persistentEditorIndex));
                m_persistentEditorIndex = QPersistentModelIndex();
            }
            // Clearing the current index also clears the details pane via
            // currentChanged. With no editor left, auto-follow resumes on the
            // next reconciliation if the view is still stick-to-bottom.
            setCurrentIndex(QModelIndex());
            setFocus();
        }

        void QConsoleWidget::keyPressEvent(QKeyEvent* event)
        {
            if (event->key() == Qt::Key_Escape &&
                (m_persistentEditorIndex.isValid() || currentIndex().isValid()))
            {
                clearTextSelection();
                event->accept();
                return;
            }
            QTableView::keyPressEvent(event);
        }

        void QConsoleWidget::changeEvent(QEvent* event)
        {
            QTableView::changeEvent(event);
            if (event->type() == QEvent::PaletteChange ||
                event->type() == QEvent::ApplicationPaletteChange)
                m_model->refreshThemeColors();
        }

        void QConsoleWidget::contextMenuEvent(QContextMenuEvent* event)
        {
            if (!m_contextMenuEnabled)
            {
                QTableView::contextMenuEvent(event);
                return;
            }
            const QModelIndex idx = indexAt(event->pos());
            if (!idx.isValid())
            {
                QTableView::contextMenuEvent(event);
                return;
            }
            const QModelIndex src = m_proxyModel->mapToSource(idx);
            const int row = src.row();
            if (row < 0 || row >= m_model->rowCount())
                return;
            const Message& msg = m_model->getElement(row);
            const QString msgText = QString::fromStdString(msg.getText());
            const LoggerID id = msg.getLoggerID();

            QMenu menu(this);
            QAction* copyText = menu.addAction("Copy message text");
            QAction* copyJson = menu.addAction("Copy row as JSON");
            menu.addSeparator();
            QAction* soloCtx = menu.addAction("Solo this context");
            QAction* hideCtx = menu.addAction("Hide this context");
            menu.addSeparator();
            QAction* hideLike = menu.addAction("Hide messages like this");

            QAction* chosen = menu.exec(event->globalPos());
            if (!chosen)
                return;
            if (chosen == copyText)
            {
                QApplication::clipboard()->setText(msgText);
            }
            else if (chosen == copyJson)
            {
                const QJsonValue v = msg.toJson();
                QJsonDocument doc;
                if (v.isObject()) doc = QJsonDocument(v.toObject());
                else if (v.isArray()) doc = QJsonDocument(v.toArray());
                else doc = QJsonDocument(QJsonObject{{"value", v}});
                QApplication::clipboard()->setText(QString::fromUtf8(doc.toJson(QJsonDocument::Indented)));
            }
            else if (chosen == soloCtx)
            {
                emit requestSoloContext(id);
            }
            else if (chosen == hideCtx)
            {
                emit requestHideContext(id);
            }
            else if (chosen == hideLike)
            {
                emit requestHideMessagesLike(msgText);
            }
        }

        void QConsoleWidget::mouseDoubleClickEvent(QMouseEvent* event)
        {
            const QModelIndex idx = indexAt(event->pos());
            if (idx.isValid())
            {
                const QModelIndex src = m_proxyModel->mapToSource(idx);
                const int row = src.row();
                if (row >= 0 && row < m_model->rowCount())
                {
                    const Message& msg = m_model->getElement(row);
                    QApplication::clipboard()->setText(QString::fromStdString(msg.getText()));
                }
            }
            QTableView::mouseDoubleClickEvent(event);
        }

        void QConsoleWidget::onNewMessage(const Message& m)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_2);
            {
                QMutexLocker locker(&m_mutex);
                m_messageQueue.push_back(m);
            }
            if (!m_flushScheduled.exchange(true))
            {
                LOGGER_RECEIVER_PROFILING_BLOCK("Emit signal: messageQued", LOGGER_COLOR_STAGE_3);
                emit messageQueued(nullptr);
            }
        }

        void QConsoleWidget::onMessageQueued(QPrivateSignal*)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_4);
            m_flushScheduled.store(false);
            // A flush already ran inside the current interval — the pending
            // timeout will pick up everything that accumulated since.
            if (m_flushTimer.isActive())
                return;
            flushQueue();
            m_flushTimer.start();
        }

        void QConsoleWidget::onFlushTimeout()
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_4);
            {
                QMutexLocker locker(&m_mutex);
                if (m_messageQueue.empty())
                    return; // idle — the next message flushes immediately
            }
            flushQueue();
            m_flushTimer.start();
        }

        void QConsoleWidget::flushQueue()
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_4);
            std::vector<Message> cpy;
            {
                QMutexLocker locker(&m_mutex);
                cpy = std::move(m_messageQueue);
                m_messageQueue.clear();
            }
            if (cpy.empty())
                return;
            // Suppress sticky updates for the entire insert-and-scroll sequence.
            // Growing the row range can trigger valueChanged (Qt clamping /
            // geometry updates) which would otherwise be mistaken for a user
            // scroll and flip the flag off. Scroll first, THEN release the guard.
            m_programmaticScroll = true;
            m_model->addLogs(std::move(cpy));
            if (m_stickToBottom && !m_persistentEditorIndex.isValid())
                scrollToBottom();
            m_programmaticScroll = false;
            emit filterChanged();
        }
    }
}
#endif
