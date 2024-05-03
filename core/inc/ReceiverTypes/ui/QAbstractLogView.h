#pragma once
#include "Logger_base.h"
#include "LoggerTypes/ContextLogger.h"
#include "ui/QContextLoggerTree.h"

#ifdef QT_WIDGETS_LIB
#include "ReceiverTypes/ui/QAbstractLogView.h"
#include <QWidget>
#include <QTreeWidget>
#include <QTimer>
#include <QCheckBox>
#include <unordered_map>
#include "LoggerTypes/ContextLogger.h"
#include "ReceiverTypes/ContextReceiver.h"
#include <QMutex>

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

            virtual void getSaveVisibleMessages(std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list) const = 0;
            bool saveVisibleMessages(const std::string &outputFile) const;

            std::vector< Logger::AbstractLogger::MetaInfo> getContexts() const;

        protected slots:
            virtual void onAllContextCheckBoxStateChanged(int state);
            virtual void on_clear_pushButton_clicked();
            virtual void on_save_pushButton_clicked();

        signals:
            void newContextAdded(QPrivateSignal*);

        private slots:
            void onLevelCheckBoxStateChangedSlot(int state);
            void onFilterTextChangedSlot(const QString& text);
            void onCheckBoxStateChangedSlot(int state);
            
            void onDateTimeFilterActivate_checkBox_stateChanged(int state);
            void onDateTimeFilterMin_changed(const DateTime& dateTime);
            void onDateTimeFilterMax_changed(const DateTime& dateTime);
            void onDateTimeFilterMinNow_pushButton_clicked();
            void onDateTimeFilterMaxNow_pushButton_clicked();
            void onDateTimeFilterType_changed(int index);

            void onNewContextAdded(QPrivateSignal*);

        protected:
            struct ContextData
            {
                QCheckBox* checkBox;
                std::shared_ptr<const Logger::AbstractLogger::MetaInfo> loggerInfo;

                ContextData(std::shared_ptr<const Logger::AbstractLogger::MetaInfo> loggerInfo,
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

            virtual void onDateTimeFilterChanged(const DateTimeFilter& filter) = 0;

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
            DateTimeFilter m_dateTimeFilter;

            bool m_autoCreateNewCheckBoxForNewContext = false;
            bool m_ignoreAllContextCheckBox_signals = false;

            struct NewContextQueueData
            {
                Logger::AbstractLogger *logger = nullptr;

                NewContextQueueData(Logger::AbstractLogger& logger)
					: logger(&logger)
				{	
                }

                NewContextQueueData(const NewContextQueueData& other)
					: logger(other.logger)
				{	
                }
                NewContextQueueData(const NewContextQueueData&& other) noexcept
                    : logger(other.logger)
                {   
                }
                
                NewContextQueueData &operator=(const NewContextQueueData& other)
				{
                    if (logger == other.logger)
                        return *this;
                    logger = other.logger;
					return *this;
				}

                void onContextDestroy(Logger::AbstractLogger& logger)
                {
					if (this->logger == &logger)
					{
						this->logger = nullptr;
					}
				}

            };

            QMutex m_newContextQueueMutex;
            std::unordered_map<Logger::AbstractLogger::LoggerID, NewContextQueueData> m_newContextQueue;
        };
    }
}
#endif