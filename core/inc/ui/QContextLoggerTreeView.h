#pragma once
#include "Logger_base.h"
#include "LoggerTypes/ContextLogger.h"
#include "ReceiverTypes/QContextLoggerTree.h"

#ifdef LOGGER_QT
#include <QWidget>
#include <QTreeWidget>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class QContextLoggerTreeView; }
QT_END_NAMESPACE

namespace Log 
{
    namespace UI
    {
        class LOGGER_EXPORT QContextLoggerTreeView : public QWidget
        {
            Q_OBJECT
        public:
            QContextLoggerTreeView(QWidget* parent = nullptr);
            ~QContextLoggerTreeView();

            void connectLogger(Logger::ContextLogger& logger);
            void disconnectLogger(Logger::ContextLogger& logger);


        private slots:
            void onUpdateTimer();
        private:

            Ui::QContextLoggerTreeView* ui;
            QTreeWidget* m_treeWidget;
            Receiver::QContextLoggerTree* m_treeItem;

            QTimer m_updateTimer;
        };
    }
}
#endif