#include "ui/QCombinedConsoleView.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogWidget.h"
#include <QVBoxLayout>

namespace Log
{
    namespace UI
    {
        QCombinedConsoleView::QCombinedConsoleView(QWidget* parent)
            : QAbstractLogWidget(parent)
        {
            setWindowTitle("Console (combined)");

            m_tabs = new QTabWidget();

            // Table tab
            m_tableWidget = new UIWidgets::QConsoleWidget();
            m_tabs->addTab(m_tableWidget, "Table");

            // Tree tab
            m_treeWidget = new QTreeWidget();
            m_treeItem = new UIWidgets::QContextLoggerTreeWidget(m_treeWidget);
            m_tabs->addTab(m_treeWidget, "Tree");

            // Vertical timeline tab
            m_verticalTimelineView = new QVerticalTimelineView();
            m_verticalTimelineView->disableSubWidget(SubWidget::settingsFrame);
            m_verticalTimelineView->disableSubWidget(SubWidget::logLevelFilter);
            m_verticalTimelineView->disableSubWidget(SubWidget::contextFilter);
            m_verticalTimelineView->disableSubWidget(SubWidget::dateTimeFilter);
            m_verticalTimelineView->disableSubWidget(SubWidget::editFrame);
            m_verticalTimelineView->setFeatureEnabled(SearchBar, false);
            m_verticalTimelineView->setFeatureEnabled(DetailsPane, false);
            m_tabs->addTab(m_verticalTimelineView, "Vertical timeline");

            // Stats tab: same treatment for QStatsConsoleView.
            m_statsView = new QStatsConsoleView();
            m_statsView->disableSubWidget(SubWidget::settingsFrame);
            m_statsView->disableSubWidget(SubWidget::logLevelFilter);
            m_statsView->disableSubWidget(SubWidget::contextFilter);
            m_statsView->disableSubWidget(SubWidget::dateTimeFilter);
            m_statsView->disableSubWidget(SubWidget::editFrame);
            m_statsView->setFeatureEnabled(SearchBar, false);
            m_statsView->setFeatureEnabled(DetailsPane, false);
            m_tabs->addTab(m_statsView, "Stats");

            setContentWidget(m_tabs);

            connect(this, &QCombinedConsoleView::messageQueued,
                    this, &QCombinedConsoleView::onMessageQueued,
                    Qt::QueuedConnection);

            connect(m_tableWidget, &UIWidgets::QConsoleWidget::filterChanged,
                    this, &QCombinedConsoleView::refreshMatchCount);
            connect(m_tableWidget, &UIWidgets::QConsoleWidget::requestSoloContext,
                    this, [this](Log::LoggerID id) { soloContext(id); });
            connect(m_tableWidget, &UIWidgets::QConsoleWidget::requestHideContext,
                    this, [this](Log::LoggerID id) { hideContext(id); });
            connect(m_tableWidget, &UIWidgets::QConsoleWidget::requestHideMessagesLike,
                    this, [this](const QString& text) {
                        setSearchTextProgrammatic(QStringLiteral("!") + text, false);
                    });
            connect(m_treeItem, &UIWidgets::QContextLoggerTreeWidget::requestSoloContext,
                    this, [this](Log::LoggerID id) { soloContext(id); });
            connect(m_treeItem, &UIWidgets::QContextLoggerTreeWidget::requestHideContext,
                    this, [this](Log::LoggerID id) { hideContext(id); });
            connect(m_treeItem, &UIWidgets::QContextLoggerTreeWidget::requestHideMessagesLike,
                    this, [this](const QString& text) {
                        setSearchTextProgrammatic(QStringLiteral("!") + text, false);
                    });
            connect(m_tableWidget, &UIWidgets::QConsoleWidget::selectionChangedMessage,
                    this, [this](const Log::Message& msg, bool has) { updateDetailsFor(msg, has); });
            connect(m_treeItem, &UIWidgets::QContextLoggerTreeWidget::selectionChangedMessage,
                    this, [this](const Log::Message& msg, bool has) { updateDetailsFor(msg, has); });

            postConstructorInit();
        }
        QCombinedConsoleView::~QCombinedConsoleView()
        {
        }

        void QCombinedConsoleView::createStaticInstance()
        {
            QCombinedConsoleView*& instancePtr = getStaticInstance();
            if (instancePtr)
                return;
            instancePtr = new QCombinedConsoleView();
        }
        void QCombinedConsoleView::destroyStaticInstance()
        {
            QCombinedConsoleView*& instancePtr = getStaticInstance();
            if (instancePtr)
            {
                delete instancePtr;
                instancePtr = nullptr;
            }
        }
        QCombinedConsoleView*& QCombinedConsoleView::getStaticInstance()
        {
            static QCombinedConsoleView* instancePtr = nullptr;
            return instancePtr;
        }

        void QCombinedConsoleView::setCurrentTab(Tab tab)
        {
            m_tabs->setCurrentIndex(static_cast<int>(tab));
        }
        QCombinedConsoleView::Tab QCombinedConsoleView::getCurrentTab() const
        {
            return static_cast<Tab>(m_tabs->currentIndex());
        }

        void QCombinedConsoleView::setFeatureEnabled(Feature f, bool enabled)
        {
            QAbstractLogWidget::setFeatureEnabled(f, enabled);
            if (f == RowContextMenu)
            {
                m_tableWidget->setContextMenuEnabled(enabled);
                m_treeItem->setContextMenuEnabled(enabled);
            }
        }
        void QCombinedConsoleView::setDateTimeFormat(DateTime::Format format)
        {
            m_tableWidget->setDateTimeFormat(format);
            m_treeItem->setDateTimeFormat(format);
        }
        DateTime::Format QCombinedConsoleView::getDateTimeFormat() const
        {
            return m_treeItem->getDateTimeFormat();
        }

        void QCombinedConsoleView::getSaveVisibleMessages(
            std::unordered_map<LoggerID, std::vector<Message>>& list) const
        {
            QMutexLocker locker(&m_mutex);
            // Save from a row-based tab (timeline/stats don't hold raw messages).
            switch (getCurrentTab())
            {
            case Tab::tree:
                m_treeItem->getSaveVisibleMessages(list);
                break;
            case Tab::table:
            default:
                m_tableWidget->getSaveVisibleMessages(list);
                break;
            }
        }
        void QCombinedConsoleView::clear()
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            {
                QMutexLocker locker(&m_mutex);
                m_messageQueue.clear();
                m_flushScheduled.store(false);
            }
            m_tableWidget->clear();
            m_treeItem->clearMessages();
            if (m_verticalTimelineView)
            {
                m_verticalTimelineView->clear();
                m_verticalTimelineView->setMode(QVerticalTimelineView::Mode::Present);
            }
            if (m_statsView) m_statsView->clear();
            QAbstractLogWidget::clear();
        }

        void QCombinedConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
            m_tableWidget->setLevelVisibility(level, isChecked);
            m_treeItem->setLevelVisibility(level, isChecked);
            if (m_verticalTimelineView) m_verticalTimelineView->setLevelEnabled(level, isChecked);
            if (m_statsView) m_statsView->setLevelEnabled(level, isChecked);
        }
        void QCombinedConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
            m_tableWidget->setContextVisibility(context.id, isChecked);
            m_treeItem->setContextVisibility(context.id, isChecked);
            if (m_verticalTimelineView) m_verticalTimelineView->setContextEnabled(context.id, isChecked);
            if (m_statsView) m_statsView->setContextEnabled(context.id, isChecked);
        }
        void QCombinedConsoleView::onDateTimeFilterChanged(const DateTimeFilter& filter)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            m_tableWidget->setDateTimeFilter(filter);
            m_treeItem->setDateTimeFilter(filter);
        }
        void QCombinedConsoleView::onSearchTextChanged(const QString& text, bool regex)
        {
            m_tableWidget->setTextFilter(text, regex);
            m_treeItem->setTextFilter(text, regex);
            if (m_verticalTimelineView) m_verticalTimelineView->setSearchTextProgrammatic(text, regex);
            if (m_statsView) m_statsView->setSearchTextProgrammatic(text, regex);
            refreshMatchCount();
        }
        int QCombinedConsoleView::matchCount() const
        {
            switch (getCurrentTab())
            {
            case Tab::tree:  return m_treeItem->getMatchCount();
            case Tab::table: return m_tableWidget->getMatchCount();
            default:         return 0; // timeline/stats: no discrete row match count
            }
        }
        void QCombinedConsoleView::findNext(bool forward)
        {
            switch (getCurrentTab())
            {
            case Tab::tree:  m_treeItem->findNext(forward); break;
            case Tab::table: m_tableWidget->findNext(forward); break;
            default:         break;
            }
        }

        void QCombinedConsoleView::onNewLogger(LogObject::Info loggerInfo)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onNewLogger(loggerInfo);
            m_tableWidget->onNewLogger(loggerInfo);
            m_treeItem->addContext(loggerInfo);
            // Live data reaches timeline/stats directly from LogManager; during
            // a file load it doesn't, so forward loaded loggers to them here.
            if (m_loading)
            {
                if (m_verticalTimelineView) m_verticalTimelineView->canvas()->addLogger(loggerInfo);
                if (m_statsView) m_statsView->ingestLoadedLogger(loggerInfo);
            }
        }
        void QCombinedConsoleView::onLoggerInfoChanged(LogObject::Info info)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLoggerInfoChanged(info);
            m_tableWidget->onNewLogger(info);
        }
        void QCombinedConsoleView::onLogMessage(Message message)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLogMessage(message);
            // Table view can receive directly (it has its own internal queue),
            // tree needs batching through the widget's queue.
            m_tableWidget->onNewMessage(message);
            {
                QMutexLocker locker(&m_mutex);
                m_messageQueue.push_back(message);
            }
            if (!m_flushScheduled.exchange(true))
                emit messageQueued(nullptr);
            // Live data reaches timeline/stats directly from LogManager; during
            // a file load it doesn't, so forward loaded messages to them here.
            if (m_loading)
            {
                if (m_verticalTimelineView) m_verticalTimelineView->canvas()->addMessage(message.getLoggerID(), message);
                if (m_statsView) m_statsView->ingestLoadedMessage(message);
            }
        }
        void QCombinedConsoleView::onChangeParent(LoggerID childID, LoggerID newParentID)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onChangeParent(childID, newParentID);
            m_treeItem->setParent(childID, newParentID);
        }

        void QCombinedConsoleView::onMessagesLoadStarted()
        {
            m_loading = true;
        }
        void QCombinedConsoleView::onMessagesLoaded()
        {
            m_loading = false;
            // Anchor the timeline to the loaded (past) data span.
            if (m_verticalTimelineView)
                m_verticalTimelineView->setMode(QVerticalTimelineView::Mode::Past);
        }

        void QCombinedConsoleView::onMessageQueued(QPrivateSignal*)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            std::vector<Message> cpy;
            {
                QMutexLocker locker(&m_mutex);
                cpy = std::move(m_messageQueue);
                m_messageQueue.clear();
            }
            m_flushScheduled.store(false);
            m_treeItem->onNewMessages(cpy);

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
