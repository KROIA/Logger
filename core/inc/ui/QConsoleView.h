#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include "ui/QConsoleWidget.h"
#include "ReceiverTypes/ui/QAbstractLogView.h"
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

            void getSaveVisibleMessages(std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list) const override;

        private slots:

        private:
            void on_clear_pushButton_clicked() override;
            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(ContextData const* context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;

            void onNewMessage(const Message& m) override;

            QConsoleWidget* m_consoleWidget;
        };
    }
}
#endif