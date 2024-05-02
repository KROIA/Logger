#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "LoggerTypes/ContextLogger.h"
#include "ui/QContextLoggerTree.h"
#include "ReceiverTypes/ui/QAbstractLogView.h"
#include <QTreeWidget>
#include <QTreeView>
#include <QMutex>

namespace Log 
{
    namespace UI
    {
        class LOGGER_EXPORT QContextLoggerTreeView: public QAbstractLogView
        {
            Q_OBJECT
        public:
            QContextLoggerTreeView(QWidget* parent = nullptr);
            ~QContextLoggerTreeView();

            void setDateTimeFormat(DateTime::Format format) override;
            DateTime::Format getDateTimeFormat() const override;

            void getSaveVisibleMessages(std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list) const override;

        private slots:

        private:
            void on_clear_pushButton_clicked() override;
            void addContext(Logger::AbstractLogger& logger) override;

            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(ContextData const* context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;
            void onNewMessage(const Message& m) override;



            QTreeWidget* m_treeWidget;
            Receiver::QContextLoggerTree* m_treeItem;
            mutable QMutex m_mutex;
        };
    }
}
#endif