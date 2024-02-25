#pragma once
#include "Logger_base.h"
#include "LoggerTypes/ContextLogger.h"
#include "ReceiverTypes/QContextLoggerTree.h"

#ifdef LOGGER_QT
#include "ui/QAbstractContextLoggerView.h"
#include <QWidget>
#include <QTreeWidget>
#include <QTimer>
#include <QCheckBox>
#include <unordered_map>

QT_BEGIN_NAMESPACE
namespace Ui { class QContextLoggerTreeView; }
QT_END_NAMESPACE

namespace Log 
{
    namespace UI
    {
        class LOGGER_EXPORT QContextLoggerTreeView : public QWidget, public QAbstractContextLoggerView
        {
            Q_OBJECT
        public:
            QContextLoggerTreeView(QWidget* parent = nullptr);
            ~QContextLoggerTreeView();

           // void connectLogger(Logger::ContextLogger& logger);
           // void disconnectLogger(Logger::ContextLogger& logger);
            void setDateTimeFormat(DateTime::Format format);
            DateTime::Format getDateTimeFormat() const;


        private slots:
            //void onUpdateTimer();

           // void onCheckBoxStateChanged(int state);
            void onAllContextCheckBoxStateChanged(int state);
            //void onLevelCheckBoxStateChanged(int state);
            //void onFilterTextChanged(const QString& text);
        private:
            //void onDeletePrivate(Logger::AbstractLogger& logger);
            //void addContextRecursive(Logger::ContextLogger& logger);
            //void onContextCreate(Logger::ContextLogger& logger);
            //void onContextDestroy(Logger::ContextLogger& logger);



            void onNewSubscribed(Logger::AbstractLogger& logger) override;
            void onUnsubscribed(Logger::AbstractLogger& logger) override;

            void onContextCreate(Logger::ContextLogger& logger) override;
            void onContextDestroy(Logger::ContextLogger& logger) override;

            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text) override;
            void onContextCheckBoxChanged(ContextData* context, bool isChecked) override;
            void onNewContextCheckBoxCreated(ContextData* context) override;
            void onContextCheckBoxDestroyed(ContextData* context) override;

            Ui::QContextLoggerTreeView* ui;
            QTreeWidget* m_treeWidget;
            Receiver::QContextLoggerTree* m_treeItem;

            //QTimer m_updateTimer;
            /*struct LoggerData
            {
                Logger::ContextLogger* logger;
                QCheckBox* checkBox;
            };
            std::unordered_map<Logger::ContextLogger*, LoggerData*> m_loggerData;

            QCheckBox *m_levelCheckBoxes[static_cast<int>(Level::__count)];*/

            bool m_ignoreAllContextCheckBox_signals = false;
        };
    }
}
#endif