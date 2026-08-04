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

            setContentWidget(m_tabs);

            connect(this, &QCombinedConsoleView::messageQueued,
                    this, &QCombinedConsoleView::onMessageQueued,
                    Qt::QueuedConnection);

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
            // Prefer the currently active tab so the user saves what they see.
            if (getCurrentTab() == Tab::tree)
                m_treeItem->getSaveVisibleMessages(list);
            else
                m_tableWidget->getSaveVisibleMessages(list);
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
            QAbstractLogWidget::clear();
        }

        void QCombinedConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
            m_tableWidget->setLevelVisibility(level, isChecked);
            m_treeItem->setLevelVisibility(level, isChecked);
        }
        void QCombinedConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
            m_tableWidget->setContextVisibility(context.id, isChecked);
            m_treeItem->setContextVisibility(context.id, isChecked);
        }
        void QCombinedConsoleView::onDateTimeFilterChanged(const DateTimeFilter& filter)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            m_tableWidget->setDateTimeFilter(filter);
            m_treeItem->setDateTimeFilter(filter);
        }

        void QCombinedConsoleView::onNewLogger(LogObject::Info loggerInfo)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onNewLogger(loggerInfo);
            m_tableWidget->onNewLogger(loggerInfo);
            m_treeItem->addContext(loggerInfo);
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
        }
        void QCombinedConsoleView::onChangeParent(LoggerID childID, LoggerID newParentID)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onChangeParent(childID, newParentID);
            m_treeItem->setParent(childID, newParentID);
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
