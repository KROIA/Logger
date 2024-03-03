#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include "ReceiverTypes/ContextReceiver.h"
#include <vector>
#include <QTableView>
#include <QStyledItemDelegate>
#include <QTimer>
#include "Utilities/QLogMessageItemModel.h"

namespace Log
{
    namespace UI
    {
        class LOGGER_EXPORT QConsoleWidget : public QTableView//, public Receiver::ContextReceiver
        {
            Q_OBJECT
            public:
			QConsoleWidget(QWidget* parent = nullptr);
			~QConsoleWidget();

            void setDateTimeFormat(DateTime::Format format);
            DateTime::Format getDateTimeFormat() const;

            void setLevelVisibility(Level level, bool isVisible);
            void setContextVisibility(Logger::AbstractLogger::LoggerID loggerID, bool isVisible);

            void onNewMessage(const Message& m);
            void clear();

        private slots:
            void onAutoScrollTimerTimeout();
            void onVertialSliderMoved(int value);
        private:
            
            /*class CustomDelegate : public QStyledItemDelegate {
            public:
                QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
            };
            */
            QLogMessageItemModel* m_model;
            QLogMessageItemProxyModel * m_proxyModel;

           /* void onNewSubscribed(Logger::AbstractLogger& logger) override;
            void onUnsubscribed(Logger::AbstractLogger& logger) override;

            void onNewMessage(const Message& m) override;
            void onClear(Logger::AbstractLogger& logger) override;
            void onDelete(Logger::AbstractLogger& logger) override;

            void onContextCreate(Logger::ContextLogger& logger) override;
            void onContextDestroy(Logger::AbstractLogger& logger) override;*/

            

            bool m_isAttaching = false;
            bool m_isDetaching = false;

            //CustomDelegate delegate;
            QTimer m_autoScrollTimer;
            
        };
    }
}
#endif