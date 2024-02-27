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
            , ContextReceiver()

        {
            m_model = new QLogMessageItemModel(this);
            m_proxyModel = new QLogMessageItemProxyModel(this);
            m_proxyModel->setSourceModel(m_model);

			setModel(m_proxyModel);
            //horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            QHeaderView* header = horizontalHeader();
            header->setStretchLastSection(true);
            header->setSectionResizeMode(QLogMessageItemModel::Column::TimeColumn, QHeaderView::Interactive);
            header->setSectionResizeMode(QLogMessageItemModel::Column::LevelColumn, QHeaderView::Interactive);
            header->setSectionResizeMode(QLogMessageItemModel::Column::ContextColumn, QHeaderView::Interactive);
            header->setSectionResizeMode(QLogMessageItemModel::Column::MessageColumn, QHeaderView::Stretch);
            
            setColumnWidth(QLogMessageItemModel::Column::TimeColumn, 150);
            setColumnWidth(QLogMessageItemModel::Column::LevelColumn, 50);
            setColumnWidth(QLogMessageItemModel::Column::ContextColumn, 100);

            //header->setSectionMinimumWid
            

            QHeaderView* vHeader = verticalHeader();
            //vHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
            vHeader->setDefaultSectionSize(10);
            vHeader->setMinimumSectionSize(15);

            connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &QConsoleWidget::onVertialSliderMoved);

            //setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
           // setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

           // setItemDelegate(&delegate);
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
        void QConsoleWidget::setContextVisibility(Logger::AbstractLogger& logger, bool isVisible)
        {
            m_proxyModel->setContextVisibility(logger, isVisible);
            m_proxyModel->invalidate(); // force update the new filter
        }

        void QConsoleWidget::clear()
        {
			m_model->clear();
		}

       /* QSize QConsoleWidget::CustomDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
        {
            Q_UNUSED(option);
            QString text = index.data(Qt::DisplayRole).toString();

            QFontMetrics fontMetrics(option.font);
            QSize textSize = fontMetrics.size(Qt::TextSingleLine, text);
            textSize.setHeight(textSize.height()*text.count('\n'));
            textSize.setHeight(50);
            // You can adjust the size as per your requirement
            return QSize(textSize.width(), textSize.height());
        }*/

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

        void QConsoleWidget::onNewSubscribed(Logger::AbstractLogger& logger)
        {
            if (m_isAttaching)
                return;
            m_isAttaching = true;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if (contextLogger)
                attachLoggerAndChilds(*contextLogger);
            m_isAttaching = false;
        }
        void QConsoleWidget::onUnsubscribed(Logger::AbstractLogger& logger)
        {
            if(m_isDetaching)
                return;
            m_isDetaching = true;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if (contextLogger)
                detachLoggerAndChilds(*contextLogger);
            m_isDetaching = false;
        }

        void QConsoleWidget::onNewMessage(const Message& m)
        {
            m_model->addLog(m);
            int count = QString::fromStdString(m.getText()).count('\n');
            if(count > 0) // Ajust row height if message has multiple lines
				setRowHeight(m_model->rowCount()-1, verticalHeader()->defaultSectionSize()*count);
        }
        void QConsoleWidget::onClear(Logger::AbstractLogger& logger)
        {

        }
        void QConsoleWidget::onDelete(Logger::AbstractLogger& logger)
        {
            if(m_isDetaching)
                return;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if(contextLogger)
                detachLoggerAndChilds(*contextLogger);
        }

        void QConsoleWidget::onContextCreate(Logger::ContextLogger& logger)
        {
            if (m_isAttaching)
                return;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if (contextLogger)
                attachLoggerAndChilds(*contextLogger);
        }
        void QConsoleWidget::onContextDestroy(Logger::ContextLogger& logger)
        {
            if (m_isDetaching)
                return;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if (contextLogger)
                detachLoggerAndChilds(*contextLogger);
        }
    }
}
#endif