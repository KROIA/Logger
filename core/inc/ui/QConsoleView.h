#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
//#include "LoggerTypes/ContextLogger.h"
#include "ui/QConsoleWidget.h"
#include "ui/QAbstractLogView.h"
#include <QTreeWidget>

namespace Log 
{
    namespace UI
    {
        class LOGGER_EXPORT QConsoleView: public QAbstractLogView
        {
            Q_OBJECT
        public:
            QConsoleView(QWidget* parent = nullptr);
            ~QConsoleView();

            void setDateTimeFormat(DateTime::Format format) override;
            DateTime::Format getDateTimeFormat() const override;

        private slots:

        private:
            void on_clear_pushButton_clicked() override;
            void onAllContextCheckBoxStateChanged(int state) override;

            void onNewSubscribed(Logger::AbstractLogger& logger) override;
            void onUnsubscribed(Logger::AbstractLogger& logger) override;

           // void onContextCreate(Logger::ContextLogger& logger) override;
           // void onContextDestroy(Logger::ContextLogger& logger) override;

            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
           // void onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text) override;
            void onContextCheckBoxChanged(ContextData* context, bool isChecked) override;
           // void onNewContextCheckBoxCreated(ContextData* context) override;
           // void onContextCheckBoxDestroyed(ContextData* context) override;

            QConsoleWidget* m_consoleWidget;
        };
    }
}
#endif