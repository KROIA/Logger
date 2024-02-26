#include "ui/QConsoleView.h"

#ifdef LOGGER_QT
#include <QHeaderView>
#include <QScrollBar>

namespace Log
{
    namespace UI
    {
        

        QConsoleView::QConsoleView(QWidget* parent)
            : QTableView(parent)
            , ContextReceiver()

        {
            m_model = new QLogMessageItemModel(this);
			setModel(m_model);
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

            connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &QConsoleView::onVertialSliderMoved);

            //setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
           // setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

           // setItemDelegate(&delegate);
            m_autoScrollTimer.setInterval(100);
            connect(&m_autoScrollTimer, &QTimer::timeout, this, &QConsoleView::onAutoScrollTimerTimeout);
            m_autoScrollTimer.start();
        }
        QConsoleView::~QConsoleView()
        {

        }

       /* QSize QConsoleView::CustomDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
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

        void QConsoleView::onAutoScrollTimerTimeout()
        {
            scrollToBottom();
        }
        void QConsoleView::onVertialSliderMoved(int value)
        {
            if(verticalScrollBar()->maximum()-value <=1)
                m_autoScrollTimer.start();
			else
				m_autoScrollTimer.stop();
        }

        void QConsoleView::onNewSubscribed(Logger::AbstractLogger& logger)
        {
            if (m_isAttaching)
                return;
            m_isAttaching = true;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if (contextLogger)
                attachLoggerAndChilds(*contextLogger);
            m_isAttaching = false;
        }
        void QConsoleView::onUnsubscribed(Logger::AbstractLogger& logger)
        {
            if(m_isDetaching)
                return;
            m_isDetaching = true;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if (contextLogger)
                detachLoggerAndChilds(*contextLogger);
            m_isDetaching = false;
        }

        void QConsoleView::onNewMessage(const Message& m)
        {
            m_model->addLog(m);
            int count = QString::fromStdString(m.getText()).count('\n');
            if(count > 0) // Ajust row height if message has multiple lines
				setRowHeight(m_model->rowCount()-1, verticalHeader()->defaultSectionSize()*count);
        }
        void QConsoleView::onClear(Logger::AbstractLogger& logger)
        {

        }
        void QConsoleView::onDelete(Logger::AbstractLogger& logger)
        {
            if(m_isDetaching)
                return;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if(contextLogger)
                detachLoggerAndChilds(*contextLogger);
        }

        void QConsoleView::onContextCreate(Logger::ContextLogger& logger)
        {
            if (m_isAttaching)
                return;
            Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
            if (contextLogger)
                attachLoggerAndChilds(*contextLogger);
        }
        void QConsoleView::onContextDestroy(Logger::ContextLogger& logger)
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