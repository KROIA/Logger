#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "ui/Widgets/QAbstractLogWidget.h"
#include "ui/Widgets/QConsoleWidget.h"
#include "ui/Widgets/QContextLoggerTreeWidget.h"
#include "ui/QStatsConsoleView.h"
#include "ui/QVerticalTimelineView.h"
#include <QTabWidget>
#include <QTreeWidget>
#include <QMutex>
#include <atomic>

namespace Log
{
    namespace UI
    {
        // Log console that exposes both the flat table view and the nested tree
        // view of the same message stream through a QTabWidget so the user can
        // switch between them without maintaining two separate views.
        class LOGGER_API QCombinedConsoleView : public UIWidgets::QAbstractLogWidget
        {
            Q_OBJECT
        public:
            enum class Tab
            {
                table        = 0,
                tree         = 1,
                verticalTime = 2,
                stats        = 3
            };

            QCombinedConsoleView(QWidget* parent = nullptr);
            ~QCombinedConsoleView();

            static void createStaticInstance();
            static void destroyStaticInstance();
            static QCombinedConsoleView*& getStaticInstance();

            void setCurrentTab(Tab tab);
            Tab getCurrentTab() const;

            void setDateTimeFormat(DateTime::Format format) override;
            DateTime::Format getDateTimeFormat() const override;
            void setFeatureEnabled(Feature f, bool enabled) override;

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const override;
            void clear() override;

        signals:
            void messageQueued(QPrivateSignal*);
        private slots:
            void onMessageQueued(QPrivateSignal*);

        private:
            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(const ContextData& context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;
            void onSearchTextChanged(const QString& text, bool regex) override;
            int matchCount() const override;
            void findNext(bool forward) override;

            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLoggerInfoChanged(LogObject::Info info) override;
            void onLogMessage(Message message) override;
            void onChangeParent(LoggerID childID, LoggerID newParentID) override;

            QTabWidget* m_tabs;
            UIWidgets::QConsoleWidget* m_tableWidget;
            QTreeWidget* m_treeWidget;
            UIWidgets::QContextLoggerTreeWidget* m_treeItem;
            QVerticalTimelineView* m_verticalTimelineView = nullptr;
            QStatsConsoleView* m_statsView = nullptr;

            mutable QMutex m_mutex;
            std::vector<Message> m_messageQueue;
            std::atomic<bool> m_flushScheduled{ false };
        };
    }
}
#endif
