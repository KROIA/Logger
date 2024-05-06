#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "ReceiverTypes/ContextReceiver.h"
#include <vector>
#include <QTableView>
#include <QStyledItemDelegate>
#include <QTimer>
#include "Utilities/QLogMessageItemModel.h"
#include <QMutex>

namespace Log
{
    namespace UI
    {
        class LOGGER_EXPORT QConsoleWidget : public QTableView
        {
            Q_OBJECT
            public:
			QConsoleWidget(QWidget* parent = nullptr);
			~QConsoleWidget();

            void setDateTimeFormat(DateTime::Format format);
            DateTime::Format getDateTimeFormat() const;

            void setLevelVisibility(Level level, bool isVisible);
            void setContextVisibility(Logger::AbstractLogger::LoggerID loggerID, bool isVisible);
            void setDateTimeFilter(const DateTimeFilter& filter);

            void onNewMessage(const Message& m);
            void clear();

            void getSaveVisibleMessages(std::vector<Log::Message::SnapshotData>& list);

        signals:
            void messageQueued(QPrivateSignal*);
        private slots:

            void onMessageQueued(QPrivateSignal*);

            void onAutoScrollTimerTimeout();
            void onVertialSliderMoved(int value);
        private:
            
            QLogMessageItemModel* m_model;
            QLogMessageItemProxyModel * m_proxyModel;

            bool m_isAttaching = false;
            bool m_isDetaching = false;
            QTimer m_autoScrollTimer;
            
            mutable QMutex m_mutex;
            std::vector<Message> m_messageQueue;
        };
    }
}
#endif