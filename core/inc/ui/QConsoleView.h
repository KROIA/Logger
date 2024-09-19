#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "ui/Widgets/QConsoleWidget.h"
#include "ui/Widgets/QAbstractLogWidget.h"
#include <QTreeWidget>
#include <memory>

namespace Log 
{
    namespace UI
    {
        class LOGGER_EXPORT QConsoleView: public UIWidgets::QAbstractLogWidget
        {
            Q_OBJECT
        public:
            QConsoleView(QWidget* parent = nullptr);
            ~QConsoleView();

            static void createStaticInstance();
			static void destroyStaticInstance();
			static QConsoleView*& getStaticInstance();

            void setDateTimeFormat(DateTime::Format format) override;
            DateTime::Format getDateTimeFormat() const override;

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const override;

        private slots:

        private:
            void on_clear_pushButton_clicked() override;
            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(const ContextData& context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;


            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLoggerInfoChanged(LogObject::Info info) override;
            void onLogMessage(Message message) override;
            void onChangeParent(LoggerID childID, LoggerID newParentID) override;

            UIWidgets::QConsoleWidget* m_consoleWidget;
        };
    }
}
#endif