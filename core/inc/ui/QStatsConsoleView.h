#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "ui/Widgets/QAbstractLogWidget.h"
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <deque>
#include <unordered_map>

namespace Log
{
    namespace UI
    {
        // Dashboard-style console view. Shows totals, per-level counters with
        // proportional bars, per-context counters, and a rolling messages/sec
        // rate. Non-scrolling overview — complements the row-based views.
        class LOGGER_API QStatsConsoleView : public UIWidgets::QAbstractLogWidget
        {
            Q_OBJECT
        public:
            QStatsConsoleView(QWidget* parent = nullptr);
            ~QStatsConsoleView();

            static void createStaticInstance();
            static void destroyStaticInstance();
            static QStatsConsoleView*& getStaticInstance();

            void setDateTimeFormat(DateTime::Format format) override;
            DateTime::Format getDateTimeFormat() const override;

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const override;
            void clear() override;

        private:
            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(const ContextData& context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;

            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLoggerInfoChanged(LogObject::Info info) override;
            void onLogMessage(Message message) override;

            void rebuildContextRow(LoggerID id);
            void refreshLevelBars();
            void refreshRate();

            DateTime::Format m_format = DateTime::Format::hourMinuteSecondMillisecond;

            QLabel* m_totalLabel = nullptr;
            QLabel* m_rateLabel = nullptr;
            QTableWidget* m_levelTable = nullptr;
            QTableWidget* m_contextTable = nullptr;
            QTimer m_rateTimer;

            size_t m_total = 0;
            size_t m_perLevel[Level::__count] = {};
            bool m_levelEnabled[Level::__count];

            struct CtxStats
            {
                std::string name;
                Color color;
                bool enabled = true;
                size_t total = 0;
                size_t perLevel[Level::__count] = {};
                int row = -1;
            };
            std::unordered_map<LoggerID, CtxStats> m_ctx;
            std::deque<qint64> m_recent; // ms epochs of last N messages for rate calc
        };
    }
}
#endif
