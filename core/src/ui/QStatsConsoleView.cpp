#include "ui/QStatsConsoleView.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogWidget.h"
#include "LogMessage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QProgressBar>
#include <QDateTime>
#include <QFont>
#include <algorithm>

namespace Log
{
    namespace UI
    {
        namespace
        {
            constexpr int kRateWindowMs = 5000;
            constexpr int kDefaultRefreshIntervalMs = 100;

            QColor levelQColor(Level lv)
            {
                return Message::getLevelColor(lv).toQColor();
            }

            QString levelName(Level lv)
            {
                return QString::fromStdString(Utilities::getLevelStr(lv));
            }

            // Creates an empty bar. Called once per table row — never per
            // message. Re-creating these per message is what made this view
            // cost ~0.5 ms/message and eventually crash under a queued burst
            // (ISS-005), because QTableWidget::setCellWidget only
            // deleteLater()s the widget it replaces.
            QProgressBar* createBar(QWidget* parent)
            {
                auto* bar = new QProgressBar(parent);
                bar->setRange(0, 1);
                bar->setValue(0);
                bar->setTextVisible(true);
                bar->setFormat("0");
                bar->setAlignment(Qt::AlignCenter);
                return bar;
            }

            // Stylesheet parsing is expensive, so this runs only when a color
            // actually changes (logger info changed, level toggled), not on
            // every value update.
            void applyBarColor(QProgressBar* bar, const QColor& color)
            {
                if (!bar)
                    return;
                // Track colors follow the app palette so the widget looks right
                // in both dark and light themes.
                QWidget* parent = bar->parentWidget();
                const QPalette pal = parent ? parent->palette() : QApplication::palette();
                const bool light = pal.color(QPalette::Base).lightness() > 128;
                const QColor track  = light ? QColor(230, 230, 230) : QColor(34, 34, 34);
                const QColor border = light ? QColor(180, 180, 180) : QColor(70, 70, 70);
                const QColor text   = light ? QColor(20, 20, 20)    : QColor(240, 240, 240);
                QString css = QString(
                    "QProgressBar { border: 1px solid %1; border-radius: 2px; background: %2;"
                    " text-align: center; color: %3; }"
                    "QProgressBar::chunk { background-color: %4; }"
                ).arg(border.name(), track.name(), text.name(), color.name());
                bar->setStyleSheet(css);
            }

            void setBarValue(QProgressBar* bar, size_t value, size_t maxValue)
            {
                if (!bar)
                    return;
                const int v = static_cast<int>(value);
                const int m = static_cast<int>(std::max<size_t>(1, maxValue));
                if (bar->maximum() != m)
                    bar->setRange(0, m);
                if (bar->value() != v)
                {
                    bar->setValue(v);
                    bar->setFormat(QString::number(v));
                }
            }
        }

        QStatsConsoleView::QStatsConsoleView(QWidget* parent)
            : QAbstractLogWidget(parent)
        {
            setWindowTitle("Console statistics");
            for (int i = 0; i < Level::__count; ++i)
                m_levelEnabled[i] = true;

            auto* content = new QWidget();
            auto* v = new QVBoxLayout(content);
            v->setContentsMargins(4, 4, 4, 4);
            v->setSpacing(6);

            // Header
            auto* header = new QWidget(content);
            auto* h = new QHBoxLayout(header);
            h->setContentsMargins(0, 0, 0, 0);
            m_totalLabel = new QLabel("Total: 0", header);
            m_rateLabel = new QLabel("Rate: 0.0 msg/s", header);
            QFont f = m_totalLabel->font();
            f.setBold(true);
            f.setPointSize(f.pointSize() + 2);
            m_totalLabel->setFont(f);
            m_rateLabel->setFont(f);
            h->addWidget(m_totalLabel);
            h->addStretch(1);
            h->addWidget(m_rateLabel);
            v->addWidget(header);

            // Level table
            auto* lvlLabel = new QLabel("By level", content);
            lvlLabel->setStyleSheet("font-weight: bold;");
            v->addWidget(lvlLabel);
            m_levelTable = new QTableWidget(Level::__count, 2, content);
            m_levelTable->setHorizontalHeaderLabels(QStringList() << "Level" << "Count");
            m_levelTable->verticalHeader()->setVisible(false);
            m_levelTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            m_levelTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            m_levelTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
            m_levelTable->setSelectionMode(QAbstractItemView::NoSelection);
            m_levelTable->setFocusPolicy(Qt::NoFocus);
            for (int i = 0; i < Level::__count; ++i)
            {
                Level lv = static_cast<Level>(i);
                auto* nameItem = new QTableWidgetItem(levelName(lv));
                nameItem->setForeground(levelQColor(lv));
                m_levelTable->setItem(i, 0, nameItem);
                m_levelBars[i] = createBar(m_levelTable);
                m_levelTable->setCellWidget(i, 1, m_levelBars[i]);
                applyLevelBarColor(i);
            }
            m_levelTable->setFixedHeight(Level::__count * 26 + m_levelTable->horizontalHeader()->height() + 4);
            v->addWidget(m_levelTable);

            // Context table
            auto* ctxLabel = new QLabel("By context", content);
            ctxLabel->setStyleSheet("font-weight: bold;");
            v->addWidget(ctxLabel);
            m_contextTable = new QTableWidget(0, 2, content);
            m_contextTable->setHorizontalHeaderLabels(QStringList() << "Context" << "Count");
            m_contextTable->verticalHeader()->setVisible(false);
            m_contextTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            m_contextTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            m_contextTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
            m_contextTable->setSelectionMode(QAbstractItemView::NoSelection);
            m_contextTable->setFocusPolicy(Qt::NoFocus);
            m_contextTable->setSortingEnabled(false);
            v->addWidget(m_contextTable, 1);

            setContentWidget(content);

            m_refreshTimer.setInterval(kDefaultRefreshIntervalMs);
            connect(&m_refreshTimer, &QTimer::timeout, this, &QStatsConsoleView::onRefreshTimeout);
            m_refreshTimer.start();

            postConstructorInit();
        }
        QStatsConsoleView::~QStatsConsoleView() {}

        void QStatsConsoleView::createStaticInstance()
        {
            QStatsConsoleView*& p = getStaticInstance();
            if (p) return;
            p = new QStatsConsoleView();
        }
        void QStatsConsoleView::destroyStaticInstance()
        {
            QStatsConsoleView*& p = getStaticInstance();
            if (p) { delete p; p = nullptr; }
        }
        QStatsConsoleView*& QStatsConsoleView::getStaticInstance()
        {
            static QStatsConsoleView* p = nullptr;
            return p;
        }

        void QStatsConsoleView::setDateTimeFormat(DateTime::Format format) { m_format = format; }
        DateTime::Format QStatsConsoleView::getDateTimeFormat() const { return m_format; }

        void QStatsConsoleView::setRefreshInterval(int intervalMs)
        {
            m_refreshTimer.setInterval(std::max(1, intervalMs));
        }
        int QStatsConsoleView::getRefreshInterval() const
        {
            return m_refreshTimer.interval();
        }

        void QStatsConsoleView::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>&) const
        {
            // Stats view has no raw message store — nothing to save.
        }
        void QStatsConsoleView::clear()
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            m_total = 0;
            for (int i = 0; i < Level::__count; ++i)
                m_perLevel[i] = 0;
            for (auto& kv : m_ctx)
            {
                kv.second.total = 0;
                for (int i = 0; i < Level::__count; ++i)
                    kv.second.perLevel[i] = 0;
            }
            m_recent.clear();
            // Clearing is a user action, so show it immediately instead of
            // waiting for the next refresh tick.
            m_dirty = true;
            refreshCounters();
            refreshRate();
            QAbstractLogWidget::clear();
        }

        void QStatsConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
        {
            QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
            if (level < Level::__count)
            {
                m_levelEnabled[level] = isChecked;
                applyLevelBarColor(static_cast<int>(level));
            }
            m_dirty = true;
        }
        void QStatsConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
        {
            QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
            auto it = m_ctx.find(context.id);
            if (it != m_ctx.end())
            {
                it->second.enabled = isChecked;
                restyleContextRow(it->second);
            }
        }
        void QStatsConsoleView::onDateTimeFilterChanged(const DateTimeFilter&) {}

        void QStatsConsoleView::onNewLogger(LogObject::Info loggerInfo)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onNewLogger(loggerInfo);
            auto& c = m_ctx[loggerInfo.id];
            c.name = loggerInfo.name;
            c.color = loggerInfo.color;
            c.enabled = loggerInfo.enabled;
            c.visibilityPolicy = loggerInfo.visibilityPolicy;
            // Invisible loggers are still tracked (so the "unknown context"
            // fallback in onLogMessage doesn't keep synthesizing a fresh stub)
            // but never get a table row.
            if (c.row < 0 && c.visibilityPolicy != ReceiverVisibilityPolicy::Invisible)
                createContextRow(loggerInfo.id);
            restyleContextRow(c);
        }
        void QStatsConsoleView::onLoggerInfoChanged(LogObject::Info info)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLoggerInfoChanged(info);
            auto it = m_ctx.find(info.id);
            if (it == m_ctx.end())
                return;
            it->second.name = info.name;
            it->second.color = info.color;
            it->second.enabled = info.enabled;
            it->second.visibilityPolicy = info.visibilityPolicy;
            restyleContextRow(it->second);
        }

        void QStatsConsoleView::onLogMessage(Message message)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLogMessage(message);

            Level lv = message.getLevel();
            LoggerID id = message.getLoggerID();

            auto it = m_ctx.find(id);
            if (it == m_ctx.end())
            {
                // Message from an unknown context — create a placeholder row.
                LogObject::Info stub;
                stub.id = id;
                stub.name = "unknown(" + std::to_string(id) + ")";
                stub.enabled = true;
                onNewLogger(stub);
                it = m_ctx.find(id);
            }

            // Invisible loggers are tracked in m_ctx (so the fallback above
            // doesn't keep re-synthesizing a stub) but excluded from every
            // stat: totals, per-level bars, per-context counts, and rate.
            if (it != m_ctx.end() && it->second.visibilityPolicy == ReceiverVisibilityPolicy::Invisible)
                return;

            ++m_total;
            if (lv < Level::__count)
                ++m_perLevel[lv];

            if (it != m_ctx.end())
            {
                ++it->second.total;
                if (lv < Level::__count)
                    ++it->second.perLevel[lv];
            }

            const qint64 now = message.getDateTime().toQDateTime().toMSecsSinceEpoch();
            m_recent.push_back(now);
            while (!m_recent.empty() && (now - m_recent.front()) > kRateWindowMs)
                m_recent.pop_front();

            // Everything above is plain counting. The widgets are updated by
            // onRefreshTimeout() so a burst of messages costs one refresh, not
            // one full widget rebuild per message.
            m_dirty = true;
        }

        void QStatsConsoleView::createContextRow(LoggerID id)
        {
            auto it = m_ctx.find(id);
            if (it == m_ctx.end() || it->second.row >= 0)
                return;
            CtxStats& c = it->second;
            c.row = m_contextTable->rowCount();
            m_contextTable->insertRow(c.row);
            c.nameItem = new QTableWidgetItem(QString::fromStdString(c.name));
            m_contextTable->setItem(c.row, 0, c.nameItem);
            c.bar = createBar(m_contextTable);
            m_contextTable->setCellWidget(c.row, 1, c.bar);
        }

        void QStatsConsoleView::restyleContextRow(const CtxStats& context)
        {
            if (context.row < 0 || !context.nameItem)
                return;
            context.nameItem->setText(QString::fromStdString(context.name));
            context.nameItem->setForeground(context.color.toQColor());
            QFont f = context.nameItem->font();
            f.setStrikeOut(!context.enabled);
            context.nameItem->setFont(f);
            applyBarColor(context.bar, context.color.toQColor());
        }

        void QStatsConsoleView::applyLevelBarColor(int levelIndex)
        {
            if (levelIndex < 0 || levelIndex >= Level::__count)
                return;
            QColor c = levelQColor(static_cast<Level>(levelIndex));
            if (!m_levelEnabled[levelIndex])
                c = c.darker(250);
            applyBarColor(m_levelBars[levelIndex], c);
        }

        void QStatsConsoleView::onRefreshTimeout()
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_2);
            refreshCounters();
            // The rate decays with wall-clock time, so it has to be recomputed
            // even when no new message arrived.
            refreshRate();
        }

        void QStatsConsoleView::refreshCounters()
        {
            if (!m_dirty)
                return;
            m_dirty = false;

            m_totalLabel->setText(QString("Total: %1").arg(m_total));

            size_t maxLevel = 1;
            for (int i = 0; i < Level::__count; ++i)
                maxLevel = std::max(maxLevel, m_perLevel[i]);
            for (int i = 0; i < Level::__count; ++i)
                setBarValue(m_levelBars[i], m_perLevel[i], maxLevel);

            // One pass for the maximum, one pass to write the values — instead
            // of the former O(contexts) scan per context per message.
            size_t maxCtx = 1;
            for (const auto& kv : m_ctx)
                maxCtx = std::max(maxCtx, kv.second.total);
            for (const auto& kv : m_ctx)
                setBarValue(kv.second.bar, kv.second.total, maxCtx);
        }

        void QStatsConsoleView::refreshRate()
        {
            const qint64 now = QDateTime::currentMSecsSinceEpoch();
            while (!m_recent.empty() && (now - m_recent.front()) > kRateWindowMs)
                m_recent.pop_front();
            double rate = m_recent.size() * 1000.0 / kRateWindowMs;
            m_rateLabel->setText(QString("Rate: %1 msg/s").arg(rate, 0, 'f', 1));
        }
    }
}
#endif
