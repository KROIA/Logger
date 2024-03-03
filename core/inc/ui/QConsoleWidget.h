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

            void onNewMessage(const Message& m);
            void clear();

        private slots:
            void onAutoScrollTimerTimeout();
            void onVertialSliderMoved(int value);
        private:
            
            QLogMessageItemModel* m_model;
            QLogMessageItemProxyModel * m_proxyModel;

            bool m_isAttaching = false;
            bool m_isDetaching = false;
            QTimer m_autoScrollTimer;
            
        };
    }
}
#endif