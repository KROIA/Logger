#include "ui/Widgets/QConsoleWidget.h"

#ifdef QT_WIDGETS_LIB
#include <QHeaderView>
#include <QScrollBar>
#include <QApplication>
#include <QThread>
#include <QClipboard>
#include <QMouseEvent>
#include <QLineEdit>
#include <QStyledItemDelegate>


namespace {
    // Read-only in-place editor: click a cell to enter text-selection mode.
    // A double-click anywhere (on the view OR on an open editor) copies the
    // whole message text of the clicked row to the clipboard. The delegate
    // installs an event filter on every editor it creates so double-click
    // works even after an editor has taken focus.
    class ReadOnlyLineEditDelegate : public QStyledItemDelegate
    {
    public:
        ReadOnlyLineEditDelegate(int messageColumn, QObject* parent)
            : QStyledItemDelegate(parent), m_messageColumn(messageColumn) {}

        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            QLineEdit* editor = new QLineEdit(parent);
            editor->setReadOnly(true);
            editor->setFrame(false);
            editor->setFont(option.font);
            editor->setAutoFillBackground(true);
            editor->setContextMenuPolicy(Qt::NoContextMenu);
            editor->setTextMargins(0, 0, 0, 0);
            editor->setContentsMargins(0, 0, 0, 0);
            editor->setAlignment(option.displayAlignment != 0 ? option.displayAlignment : (Qt::AlignLeft | Qt::AlignVCenter));

            QPalette pal = editor->palette();
            const QVariant bg = index.data(Qt::BackgroundRole);
            if (bg.isValid() && bg.canConvert<QBrush>())
            {
                const QColor bgc = bg.value<QBrush>().color();
                if (bgc.isValid())
                {
                    pal.setColor(QPalette::Base, bgc);
                    pal.setColor(QPalette::Window, bgc);
                }
            }
            const QVariant fg = index.data(Qt::ForegroundRole);
            if (fg.isValid() && fg.canConvert<QBrush>())
            {
                const QColor fgc = fg.value<QBrush>().color();
                if (fgc.isValid() && fgc.alpha() > 0)
                    pal.setColor(QPalette::Text, fgc);
            }
            // Keep selection highlight visible even when the editor loses focus
            // (Qt would otherwise switch to the Inactive palette group, which on
            // most themes renders the selection as invisible or nearly so).
            pal.setColor(QPalette::Inactive, QPalette::Highlight,
                pal.color(QPalette::Active, QPalette::Highlight));
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                pal.color(QPalette::Active, QPalette::HighlightedText));
            editor->setPalette(pal);

            editor->setText(index.data(Qt::DisplayRole).toString());
            editor->setProperty("__persistentIndex", QVariant::fromValue(QPersistentModelIndex(index)));
            editor->installEventFilter(const_cast<ReadOnlyLineEditDelegate*>(this));
            return editor;
        }
        void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const override {}
        // Preserve the QLineEdit's text selection and cursor across model
        // refreshes (Qt calls setEditorData on persistent editors whenever it
        // thinks the underlying data may have changed).
        void setEditorData(QWidget* editor, const QModelIndex& index) const override
        {
            QLineEdit* le = qobject_cast<QLineEdit*>(editor);
            if (!le)
            {
                QStyledItemDelegate::setEditorData(editor, index);
                return;
            }
            const QString newText = index.data(Qt::DisplayRole).toString();
            if (le->text() == newText)
                return;
            const int selStart = le->selectionStart();
            const int selLen = le->selectedText().length();
            const int cursor = le->cursorPosition();
            const bool wasBlocked = le->blockSignals(true);
            le->setText(newText);
            if (selStart >= 0 && selLen > 0)
                le->setSelection(selStart, selLen);
            else
                le->setCursorPosition(qMin(cursor, newText.length()));
            le->blockSignals(wasBlocked);
        }
        void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override
        {
            editor->setGeometry(option.rect);
        }

        bool eventFilter(QObject* obj, QEvent* ev) override
        {
            if (ev->type() == QEvent::MouseButtonDblClick)
            {
                QLineEdit* editor = qobject_cast<QLineEdit*>(obj);
                if (editor)
                {
                    QPersistentModelIndex idx = editor->property("__persistentIndex").value<QPersistentModelIndex>();
                    if (idx.isValid())
                    {
                        const QString text = idx.model()->index(idx.row(), m_messageColumn, idx.parent())
                            .data(Qt::DisplayRole).toString();
                        if (!text.isEmpty())
                            QApplication::clipboard()->setText(text);
                        // Consume so QLineEdit's word-select doesn't fight the copy gesture.
                        return true;
                    }
                }
            }
            return QStyledItemDelegate::eventFilter(obj, ev);
        }
    private:
        int m_messageColumn;
    };
}


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

            connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &QConsoleWidget::onVertialSliderMoved);
            m_autoScrollTimer.setInterval(100);
            connect(&m_autoScrollTimer, &QTimer::timeout, this, &QConsoleWidget::onAutoScrollTimerTimeout);
            m_autoScrollTimer.start();

            connect(this, &QConsoleWidget::messageQueued, this, &QConsoleWidget::onMessageQueued, Qt::QueuedConnection);

            // In-cell text selection: single click on a cell opens a persistent
            // read-only line editor over it, so users can drag-select individual
            // text. The editor persists across model updates (new messages don't
            // clear the user's selection).
            setItemDelegate(new ReadOnlyLineEditDelegate(QLogMessageItemModel::Column::MessageColumn, this));
            setEditTriggers(QAbstractItemView::NoEditTriggers); // we open editors manually in currentChanged
            setSelectionMode(QAbstractItemView::NoSelection);   // no row highlight — user selects text, not rows
            setTextElideMode(Qt::ElideNone);
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
            m_proxyModel->setTextFilter(text, useRegex);
        }

        void QConsoleWidget::onNewLogger(const LogObject::Info& info)
        {
            m_model->setLoggerInfo(info);
        }
        void QConsoleWidget::clear()
        {
            {
                QMutexLocker locker(&m_mutex);
                m_messageQueue.clear();
                m_flushScheduled.store(false);
            }
            m_model->clear();
            m_model->clearLoggerCache();
        }
        void QConsoleWidget::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list)
        {
            if (QApplication::instance() && QApplication::instance()->thread() != QThread::currentThread())
            {  }
            else
                onMessageQueued(nullptr);

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
            scrollToBottom();
        }

        void QConsoleWidget::onVertialSliderMoved(int value)
        {
            if (verticalScrollBar()->maximum() - value <= 1)
                m_autoScrollTimer.start();
            else
                m_autoScrollTimer.stop();
        }

        void QConsoleWidget::resizeEvent(QResizeEvent* event)
        {
            QTableView::resizeEvent(event);
            // Resize the row height to fit the text
            this->resizeRowsToContents();
        }

        void QConsoleWidget::currentChanged(const QModelIndex& current, const QModelIndex& previous)
        {
            QTableView::currentChanged(current, previous);
            if (m_persistentEditorIndex.isValid())
            {
                closePersistentEditor(QModelIndex(m_persistentEditorIndex));
            }
            if (current.isValid())
            {
                openPersistentEditor(current);
                m_persistentEditorIndex = QPersistentModelIndex(current);
            }
            else
            {
                m_persistentEditorIndex = QPersistentModelIndex();
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
            std::vector<Message> cpy;
            {
                QMutexLocker locker(&m_mutex);
                cpy = std::move(m_messageQueue);
                m_messageQueue.clear();
            }
            m_model->addLogs(std::move(cpy));

            m_flushScheduled.store(false);
            bool needsReschedule = false;
            {
                QMutexLocker locker(&m_mutex);
                needsReschedule = !m_messageQueue.empty();
            }
            if (needsReschedule && !m_flushScheduled.exchange(true))
                emit messageQueued(nullptr);
        }
    }
}
#endif
