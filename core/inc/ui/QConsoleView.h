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
        class LOGGER_EXPORT QConsoleView : public QTableView, public Receiver::ContextReceiver
        {
            Q_OBJECT
            public:
			QConsoleView(QWidget* parent = nullptr);
			~QConsoleView();

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

            void onNewSubscribed(Logger::AbstractLogger& logger) override;
            void onUnsubscribed(Logger::AbstractLogger& logger) override;

            void onNewMessage(const Message& m) override;
            void onClear(Logger::AbstractLogger& logger) override;
            void onDelete(Logger::AbstractLogger& logger) override;

            void onContextCreate(Logger::ContextLogger& logger) override;
            void onContextDestroy(Logger::ContextLogger& logger) override;

            bool m_isAttaching = false;
            bool m_isDetaching = false;

            //CustomDelegate delegate;
            QTimer m_autoScrollTimer;
        };
    }
}
#endif