#pragma once
#include "Logger_base.h"
#include "LoggerTypes/ContextLogger.h"
#include "ReceiverTypes/QContextLoggerTree.h"

#ifdef LOGGER_QT
#include "ui/QAbstractLogView.h"
#include <QWidget>
#include <QTreeWidget>
#include <QTimer>
#include <QCheckBox>
#include <unordered_map>
#include "LoggerTypes/ContextLogger.h"
#include "ReceiverTypes/ContextReceiver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QAbstractLogView; }
QT_END_NAMESPACE

namespace Log 
{
    namespace UI
    {
        class QAbstractLogView;
        class LOGGER_EXPORT QSignalHandler : public QObject
        {

            Q_OBJECT
        public slots:
            void onLevelCheckBoxStateChangedSlot(int state);
            void onFilterTextChangedSlot(const QString& text);
            void onCheckBoxStateChangedSlot(int state);

        public:
            QAbstractLogView* m_parent;
        };

        class LOGGER_EXPORT QAbstractLogView : public QWidget, public Receiver::ContextReceiver
        {
            Q_OBJECT
            friend class QSignalHandler;
        public:
            QAbstractLogView(QWidget* parent = nullptr);
            ~QAbstractLogView();

           // void connectLogger(Logger::ContextLogger& logger);
           // void disconnectLogger(Logger::ContextLogger& logger);
            virtual void setDateTimeFormat(DateTime::Format format) = 0;
            virtual DateTime::Format getDateTimeFormat() const = 0;


        protected slots:
            //void onUpdateTimer();

           // void onCheckBoxStateChanged(int state);
            virtual void onAllContextCheckBoxStateChanged(int state);
            virtual void on_clear_pushButton_clicked() = 0;
            //void onLevelCheckBoxStateChanged(int state);
            //void onFilterTextChanged(const QString& text);
        protected:
            //void onDeletePrivate(Logger::AbstractLogger& logger);
            //void addContextRecursive(Logger::ContextLogger& logger);
            //void onContextCreate(Logger::ContextLogger& logger);
            //void onContextDestroy(Logger::ContextLogger& logger);
            struct ContextData
            {
                Logger::AbstractLogger* logger;
                QCheckBox* checkBox;

                ContextData()
                    : logger(nullptr), checkBox(nullptr)
                {}
            };
            void setContentWidget(QWidget* widget);

            void onNewSubscribed(Logger::AbstractLogger& logger) override;
            void onUnsubscribed(Logger::AbstractLogger& logger) override;

            void onContextCreate(Logger::ContextLogger& logger) override;
            void onContextDestroy(Logger::ContextLogger& logger) override;

            virtual void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked);
            virtual void onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text);
            virtual void onContextCheckBoxChanged(ContextData* context, bool isChecked);
            virtual void onNewContextCheckBoxCreated(ContextData* context);
            virtual void onContextCheckBoxDestroyed(ContextData* context);

            //-------------
          



            void onNewMessage(const Message& m) override;
            void onClear(Logger::AbstractLogger& logger) override;
            void onDelete(Logger::AbstractLogger& logger) override;

            virtual void addContextRecursive(Logger::ContextLogger& logger);


            Ui::QAbstractLogView* ui;
        private:
            QCheckBox* m_levelCheckBoxes[static_cast<int>(Level::__count)];
            std::vector<QLineEdit*> m_filterTextEdits;
            std::unordered_map<Logger::AbstractLogger*, ContextData*> m_contextData;

            bool m_autoCreateNewCheckBoxForNewContext = false;
            QSignalHandler m_signalHandler;
            //------------------
            
            //QTreeWidget* m_treeWidget;
            //Receiver::QContextLoggerTree* m_treeItem;

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