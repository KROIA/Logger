#include "ui/QStatsConsoleView.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogWidget.h"
#include "LogMessage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
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

            QColor levelQColor(Level lv)
            {
                return Message::getLevelColor(lv).toQColor();
            }

            QString levelName(Level lv)
            {
                return QString::fromStdString(Utilities::getLevelStr(lv));
            }

            QWidget* makeBar(int value, int maxValue, const QColor& color, QWidget* parent)
            {
                auto* bar = new QProgressBar(parent);
                bar->setRange(0, std::max(1, maxValue));
                bar->setValue(value);
                bar->setTextVisible(true);
                bar->setFormat(QString::number(value));
                bar->setAlignment(Qt::AlignCenter);
                QString css = QString(
                    "QProgressBar { border: 1px solid #444; border-radius: 2px; background: #222; text-align: center; color: white; }"
                    "QProgressBar::chunk { background-color: %1; }"
                ).arg(color.name());
                bar->setStyleSheet(css);
                return bar;
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
                m_levelTable->setCellWidget(i, 1, makeBar(0, 1, levelQColor(lv), m_levelTable));
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

            m_rateTimer.setInterval(500);
            connect(&m_rateTimer, &QTimer::timeout, this, &QStatsConsoleView::refreshRate);
            m_rateTimer.start();

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
                rebuildContextRow(kv.first);
            }
            m_recent.clear();
            m_totalLabel->setText("Total: 0");
            refreshLevelBars();
            refreshRate();
            QAbstractLogWidget::clear();
        }

        void QStatsConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
        {
            QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
            if (level < Level::__count)
                m_levelEnabled[level] = isChecked;
            refreshLevelBars();
        }
        void QStatsConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
        {
            QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
            auto it = m_ctx.find(context.id);
            if (it != m_ctx.end())
            {
                it->second.enabled = isChecked;
                rebuildContextRow(context.id);
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
            if (c.row < 0)
            {
                c.row = m_contextTable->rowCount();
                m_contextTable->insertRow(c.row);
            }
            rebuildContextRow(loggerInfo.id);
        }
        void QStatsConsoleView::onLoggerInfoChanged(LogObject::Info info)
        {
            QAbstractLogWidget::onLoggerInfoChanged(info);
            auto it = m_ctx.find(info.id);
            if (it == m_ctx.end())
                return;
            it->second.name = info.name;
            it->second.color = info.color;
            it->second.enabled = info.enabled;
            rebuildContextRow(info.id);
        }

        void QStatsConsoleView::onLogMessage(Message message)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLogMessage(message);

            Level lv = message.getLevel();
            LoggerID id = message.getLoggerID();

            ++m_total;
            if (lv < Level::__count)
                ++m_perLevel[lv];

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
            if (it != m_ctx.end())
            {
                ++it->second.total;
                if (lv < Level::__count)
                    ++it->second.perLevel[lv];
                rebuildContextRow(id);
            }

            const qint64 now = message.getDateTime().toQDateTime().toMSecsSinceEpoch();
            m_recent.push_back(now);
            while (!m_recent.empty() && (now - m_recent.front()) > kRateWindowMs)
                m_recent.pop_front();

            m_totalLabel->setText(QString("Total: %1").arg(m_total));
            refreshLevelBars();
        }

        void QStatsConsoleView::rebuildContextRow(LoggerID id)
        {
            auto it = m_ctx.find(id);
            if (it == m_ctx.end() || it->second.row < 0)
                return;
            const CtxStats& c = it->second;
            int row = c.row;

            auto* nameItem = new QTableWidgetItem(QString::fromStdString(c.name));
            nameItem->setForeground(c.color.toQColor());
            if (!c.enabled)
            {
                QFont f = nameItem->font();
                f.setStrikeOut(true);
                nameItem->setFont(f);
            }
            m_contextTable->setItem(row, 0, nameItem);

            size_t maxVal = 1;
            for (const auto& kv : m_ctx)
                maxVal = std::max(maxVal, kv.second.total);
            m_contextTable->setCellWidget(row, 1,
                makeBar(static_cast<int>(c.total), static_cast<int>(maxVal), c.color.toQColor(), m_contextTable));
        }

        void QStatsConsoleView::refreshLevelBars()
        {
            size_t maxVal = 1;
            for (int i = 0; i < Level::__count; ++i)
                maxVal = std::max(maxVal, m_perLevel[i]);
            for (int i = 0; i < Level::__count; ++i)
            {
                Level lv = static_cast<Level>(i);
                QColor c = levelQColor(lv);
                if (!m_levelEnabled[i])
                    c = c.darker(250);
                m_levelTable->setCellWidget(i, 1,
                    makeBar(static_cast<int>(m_perLevel[i]), static_cast<int>(maxVal), c, m_levelTable));
            }
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
