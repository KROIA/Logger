#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include "LoggerTypes/ContextLogger.h"
#include "ReceiverTypes/ContextReceiver.h"
#include <QObject>
#include <QCheckBox>
#include <QLineEdit>
#include <unordered_map>
#include <vector>

namespace Log
{
    namespace UI
    {
        class QAbstractContextLoggerView;
        class LOGGER_EXPORT QSignalHandler : public QObject
        {

            Q_OBJECT
        public slots:
            void onLevelCheckBoxStateChangedSlot(int state);
            void onFilterTextChangedSlot(const QString& text);
            void onCheckBoxStateChangedSlot(int state);

        public:
            QAbstractContextLoggerView* m_parent;
        };

        class LOGGER_EXPORT QAbstractContextLoggerView : public Receiver::ContextReceiver
        {
            friend class QSignalHandler;
        public:
            

            QAbstractContextLoggerView(QObject* parent = nullptr);
            ~QAbstractContextLoggerView();


        protected:
            void setupComponents(QWidget* parent,
                const std::vector< QLineEdit*>& filterTextEdits,
                QLayout *levelCheckButtonContainer,
                bool autoCreateNewCheckBoxForNewContext);

            struct ContextData
            {
                Logger::ContextLogger* logger;
                QCheckBox* checkBox;

                ContextData()
                    : logger(nullptr), checkBox(nullptr)
                {}
            };

            void onNewSubscribed(Logger::AbstractLogger& logger) override;
            void onUnsubscribed(Logger::AbstractLogger& logger) override;

            void onContextCreate(Logger::ContextLogger& logger) override;
            void onContextDestroy(Logger::ContextLogger& logger) override;

            

            void onNewMessage(const Message& m) override;
            void onClear(Logger::AbstractLogger& logger) override;
            void onDelete(Logger::AbstractLogger& logger) override;

            virtual void addContextRecursive(Logger::ContextLogger& logger);

            virtual void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) = 0;
            virtual void onFilterTextChanged(size_t index, QLineEdit*lineEdit, const std::string& text) = 0;
            virtual void onContextCheckBoxChanged(ContextData* context, bool isChecked) = 0;
            virtual void onNewContextCheckBoxCreated(ContextData* context) = 0;
            virtual void onContextCheckBoxDestroyed(ContextData* context) = 0;
        
            QCheckBox* m_levelCheckBoxes[static_cast<int>(Level::__count)];
            std::vector<QLineEdit*> m_filterTextEdits;
            std::unordered_map<Logger::ContextLogger*, ContextData*> m_contextData;
        
            bool m_autoCreateNewCheckBoxForNewContext = false;
        private:
            

            QSignalHandler m_signalHandler;
            QWidget *m_parent;
            //void onDeletePrivate(Logger::AbstractLogger& logger);
            //void addContextRecursive(Logger::ContextLogger& logger);
            

            //QTimer m_updateTimer;
            /*struct LoggerData
            {
                Logger::ContextLogger* logger;
                QCheckBox* checkBox;
            };
            std::unordered_map<Logger::ContextLogger*, LoggerData*> m_loggerData;

            QCheckBox* m_levelCheckBoxes[static_cast<int>(Level::__count)];

            bool m_ignoreAllContextCheckBox_signals = false;*/
        };
    }
}

#endif