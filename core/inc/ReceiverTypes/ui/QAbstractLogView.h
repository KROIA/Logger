#pragma once
#include "Logger_base.h"
#include "LoggerTypes/ContextLogger.h"
#include "ReceiverTypes/ui/QContextLoggerTree.h"

#ifdef LOGGER_QT
#include "ReceiverTypes/ui/QAbstractLogView.h"
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
        class LOGGER_EXPORT QAbstractLogView : public QWidget, public Receiver::ContextReceiver
        {
            Q_OBJECT
            friend class QSignalHandler;
        public:
            QAbstractLogView(QWidget* parent = nullptr);
            ~QAbstractLogView();

            virtual void setDateTimeFormat(DateTime::Format format) = 0;
            virtual DateTime::Format getDateTimeFormat() const = 0;


        protected slots:
            virtual void onAllContextCheckBoxStateChanged(int state);
            virtual void on_clear_pushButton_clicked();

        private slots:
            void onLevelCheckBoxStateChangedSlot(int state);
            void onFilterTextChangedSlot(const QString& text);
            void onCheckBoxStateChangedSlot(int state);

        protected:
            struct ContextData
            {
                QCheckBox* checkBox;
                std::shared_ptr<const Logger::AbstractLogger::LoggerMetaInfo> loggerInfo;

                ContextData(std::shared_ptr<const Logger::AbstractLogger::LoggerMetaInfo> loggerInfo,
                    QCheckBox* checkBox)
                    : checkBox(checkBox)
                    , loggerInfo(loggerInfo)
                {
                    checkBox->setChecked(true);
                    checkBox->setText(loggerInfo->name.c_str());
                }
            };
            void setContentWidget(QWidget* widget);

            void onNewSubscribed(Logger::AbstractLogger& logger) override;
            void onUnsubscribed(Logger::AbstractLogger& logger) override;

            void onContextCreate(Logger::ContextLogger& logger) override;
            void onContextDestroy(Logger::AbstractLogger& logger) override;
            virtual void addContext(Logger::AbstractLogger& logger);

            virtual void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked);
            virtual void onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text);
            virtual void onContextCheckBoxChanged(ContextData const* context, bool isChecked);
            virtual void onNewContextCheckBoxCreated(ContextData const* context);
            virtual void onContextCheckBoxDestroyed(ContextData const* context);

            virtual void removeContext(Logger::AbstractLogger::LoggerID id);
          



            void onNewMessage(const Message& m) = 0;
            void onClear(Logger::AbstractLogger& logger) override;
            void onDelete(Logger::AbstractLogger& logger) override;

            virtual void addContextRecursive(Logger::ContextLogger& logger);
            virtual void addChildContextRecursive(Logger::ContextLogger& logger);


            Ui::QAbstractLogView* ui;
        private:
            QCheckBox* m_levelCheckBoxes[static_cast<int>(Level::__count)];
            std::vector<QLineEdit*> m_filterTextEdits;
            std::unordered_map<Logger::AbstractLogger::LoggerID, ContextData*> m_contextData;

            bool m_autoCreateNewCheckBoxForNewContext = false;
            bool m_ignoreAllContextCheckBox_signals = false;
        };
    }
}
#endif