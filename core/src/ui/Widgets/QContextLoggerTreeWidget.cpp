#include "ui/Widgets/QConsoleWidget.h"

#ifdef QT_WIDGETS_LIB
#include <QHeaderView>
#include <QScrollBar>
#include <QApplication>
#include <QThread>


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
            m_proxyModel->invalidate(); // force update the new filter
        }
        void QConsoleWidget::setContextVisibility(LoggerID loggerID, bool isVisible)
        {
            m_proxyModel->setContextVisibility(loggerID, isVisible);
            m_proxyModel->invalidate(); // force update the new filter
        }
        void QConsoleWidget::setDateTimeFilter(const DateTimeFilter& filter)
        {
            m_proxyModel->setDateTimeFilter(filter);
        }

        void QConsoleWidget::clear()
        {
            m_model->clear();
        }
        void QConsoleWidget::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list)
        {
            if (QApplication::instance(), QApplication::instance()->thread() != QThread::currentThread())
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

        void QConsoleWidget::onNewMessage(const Message& m)
        {
            {
                QMutexLocker locker(&m_mutex);
                m_messageQueue.push_back(m);
            }
            if (QApplication::instance(), QApplication::instance()->thread() != QThread::currentThread())
            {
                emit messageQueued(nullptr);
            }
            else
                onMessageQueued(nullptr);
        }

        void QConsoleWidget::onMessageQueued(QPrivateSignal*)
        {
            std::vector<Message> cpy;
            {
                QMutexLocker locker(&m_mutex);
                cpy = m_messageQueue;
                m_messageQueue.clear();
            }
            QMutexLocker locker(&m_mutex);
            for (auto& m : cpy)
                m_model->addLog(m);
        }
    }
}
#endif