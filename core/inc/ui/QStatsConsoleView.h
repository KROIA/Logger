#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "ui/Widgets/QAbstractLogWidget.h"
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <deque>
#include <unordered_map>

class QProgressBar;

namespace Log
{
    namespace UI
    {
        // Dashboard-style console view. Shows totals, per-level counters with
        // proportional bars, per-context counters, and a rolling messages/sec
        // rate. Non-scrolling overview — complements the row-based views.
        //
        // Message handling is coalesced: onLogMessage only updates counters and
        // raises a dirty flag; the widgets are refreshed from a timer. The bar
        // widgets are created once per row and afterwards only get new values,
        // never re-created (see ISS-005).
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

            // Interval in ms at which the counters are pushed into the widgets.
            // Counting itself is always immediate; only the display is delayed.
            void setRefreshInterval(int intervalMs);
            int getRefreshInterval() const;

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const override;
            void clear() override;

            // Public feed points for composed views (e.g. QCombinedConsoleView)
            // that need to push file-loaded data into the stats view, which
            // otherwise only receives live data from LogManager.
            void ingestLoadedLogger(const LogObject::Info& info) { onNewLogger(info); }
            void ingestLoadedMessage(const Message& message) { onLogMessage(message); }

        private:
            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(const ContextData& context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;

            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLoggerInfoChanged(LogObject::Info info) override;
            void onLogMessage(Message message) override;

            struct CtxStats
            {
                std::string name;
                Color color;
                bool enabled = true;
                ReceiverVisibilityPolicy visibilityPolicy = ReceiverVisibilityPolicy::AutoVisible;
                size_t total = 0;
                size_t perLevel[Level::__count] = {};
                int row = -1;
                QTableWidgetItem* nameItem = nullptr;
                QProgressBar* bar = nullptr;
            };

            // Creates the row, its name item and its bar — once per context.
            void createContextRow(LoggerID id);
            // Pushes name / color / strike-out from the stored Info into the
            // already existing row widgets. Called on logger lifecycle events
            // only, not per message.
            void restyleContextRow(const CtxStats& context);
            // Timer slot: writes the counters into the widgets if anything
            // changed, and always updates the rolling rate.
            void onRefreshTimeout();
            void refreshCounters();
            void refreshRate();
            void applyLevelBarColor(int levelIndex);

            DateTime::Format m_format = DateTime::Format::hourMinuteSecondMillisecond;

            QLabel* m_totalLabel = nullptr;
            QLabel* m_rateLabel = nullptr;
            QTableWidget* m_levelTable = nullptr;
            QTableWidget* m_contextTable = nullptr;
            QTimer m_refreshTimer;
            bool m_dirty = false;

            size_t m_total = 0;
            size_t m_perLevel[Level::__count] = {};
            bool m_levelEnabled[Level::__count];
            QProgressBar* m_levelBars[Level::__count] = {};

            std::unordered_map<LoggerID, CtxStats> m_ctx;
            std::deque<qint64> m_recent; // ms epochs of last N messages for rate calc
        };
    }
}
#endif
