#pragma once
#include "Logger_base.h"
#include "Utilities/DateTime.h"
#include "ui/QContextLoggerTree.h"
#include "LogMessage.h"
#include "ReceiverTypes/AbstractReceiver.h"

#ifdef QT_WIDGETS_LIB
#include "ReceiverTypes/ui/QAbstractLogView.h"
#include <QWidget>
#include <QTreeWidget>
#include <QTimer>
#include <QCheckBox>
#include <unordered_map>
#include <QMutex>

QT_BEGIN_NAMESPACE
namespace Ui { class QAbstractLogView; }
QT_END_NAMESPACE

namespace Log 
{
    namespace UI
    {
        class LOGGER_EXPORT QAbstractLogView : public QWidget, public AbstractReceiver
        {
            Q_OBJECT
            friend class QSignalHandler;

        
        public:
            QAbstractLogView(QWidget* parent = nullptr);
            ~QAbstractLogView();

        protected:
            void postConstructorInit();

            virtual void setDateTimeFormat(DateTime::Format format) = 0;
            virtual DateTime::Format getDateTimeFormat() const = 0;

            virtual void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const = 0;
            bool saveVisibleMessages(const std::string &outputFile) const;

        protected slots:
            virtual void onAllContextCheckBoxStateChanged(int state);
            virtual void on_clear_pushButton_clicked();
            virtual void on_save_pushButton_clicked();

        

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

        protected:

            struct ContextData
            {
                LoggerID id;
                std::vector<Message> messages;
                QCheckBox* checkBox = nullptr;
            };

            
            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLoggerInfoChanged(LogObject::Info info) override;
            void onLogMessage(Message message) override;
            void onChangeParent(LoggerID childID, LoggerID newParentID) override;


            void setContentWidget(QWidget* widget);

            virtual void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked);
            virtual void onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text);
            virtual void onContextCheckBoxChanged(const ContextData& context, bool isChecked);
            virtual void onDateTimeFilterChanged(const DateTimeFilter& filter) = 0;


            Ui::QAbstractLogView* ui;
        private:
            QCheckBox* m_levelCheckBoxes[static_cast<int>(Level::__count)];
            std::vector<QLineEdit*> m_filterTextEdits;

            std::unordered_map<LoggerID, ContextData> m_contextData;
            DateTimeFilter m_dateTimeFilter;

            bool m_autoCreateNewCheckBoxForNewContext = false;
            bool m_ignoreAllContextCheckBox_signals = false;
        };
    }
}
#endif