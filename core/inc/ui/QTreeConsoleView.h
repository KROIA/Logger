#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "ui/Widgets/QContextLoggerTreeWidget.h"
#include "ui/Widgets/QAbstractLogWidget.h"
#include <QTreeWidget>
#include <QTreeView>
#include <QMutex>

namespace Log 
{
    namespace UI
    {
        class LOGGER_EXPORT QTreeConsoleView: public UIWidgets::QAbstractLogWidget
        {
            Q_OBJECT
        public:
            QTreeConsoleView(QWidget* parent = nullptr);
            ~QTreeConsoleView();

            static void createStaticInstance();
            static void destroyStaticInstance();
            static QTreeConsoleView*& getStaticInstance();

            void setDateTimeFormat(DateTime::Format format) override;
            DateTime::Format getDateTimeFormat() const override;

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const override;

        signals:
                void messageQueued(QPrivateSignal*);
        private slots:

            void onMessageQueued(QPrivateSignal*);

        private:
            void on_clear_pushButton_clicked() override;
    

            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(const ContextData& context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;

            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLogMessage(Message message) override;
            void onChangeParent(LoggerID childID, LoggerID newParentID) override;



            QTreeWidget* m_treeWidget;
            UIWidgets::QContextLoggerTreeWidget* m_treeItem;
            mutable QMutex m_mutex;

            
            std::vector<Message> m_messageQueue;
        };
        
    }
}
#endif