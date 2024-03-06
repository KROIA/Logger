#include "ui/QConsoleWidget.h"

#ifdef LOGGER_QT
#include <QHeaderView>
#include <QScrollBar>

namespace Log
{
    namespace UI
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
        void QConsoleWidget::setContextVisibility(Logger::AbstractLogger::LoggerID loggerID, bool isVisible)
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
        void QConsoleWidget::getSaveVisibleMessages(std::vector<Log::Message::SnapshotData>& list) const
        {
            int count = m_model->rowCount();
            list.reserve(count);
            for (int i = 0; i < count; ++i)
            {
                if (m_proxyModel->filterAcceptsRow(i))
                {
					const Message::SnapshotData& data = m_model->getElement(i);
                    list.push_back(data);
				}
			}
        }

        void QConsoleWidget::onAutoScrollTimerTimeout()
        {
            scrollToBottom();
        }
        
        void QConsoleWidget::onVertialSliderMoved(int value)
        {
            if(verticalScrollBar()->maximum()-value <=1)
                m_autoScrollTimer.start();
			else
				m_autoScrollTimer.stop();
        }

        void QConsoleWidget::onNewMessage(const Message& m)
        {
            m_model->addLog(m);
            int count = QString::fromStdString(m.getText()).count('\n');
            if(count > 0) // Ajust row height if message has multiple lines
				setRowHeight(m_model->rowCount()-1, verticalHeader()->defaultSectionSize()*count);
        }
    }
}
#endif