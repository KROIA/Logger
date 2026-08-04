#include "ui/QTimelineConsoleView.h"

#ifdef QT_WIDGETS_LIB
#include "LogMessage.h"

#include <QPainter>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolTip>
#include <QHelpEvent>
#include <algorithm>
#include <cmath>

namespace Log
{
    namespace UI
    {
        // ----- QTimelineCanvas ------------------------------------------------

        QTimelineCanvas::QTimelineCanvas(QWidget* parent)
            : QWidget(parent)
        {
            setMinimumHeight(120);
            setMouseTracking(true);
            for (int i = 0; i < Level::__count; ++i)
                m_levelEnabled[i] = true;
            setAttribute(Qt::WA_StyledBackground, false);
            setAutoFillBackground(true);
            QPalette p = palette();
            p.setColor(QPalette::Window, QColor(30, 30, 30));
            setPalette(p);
        }

        qint64 QTimelineCanvas::viewEndMs() const
        {
            if (m_followLive)
                return QDateTime::currentMSecsSinceEpoch();
            return m_frozenEndMs;
        }
        qint64 QTimelineCanvas::viewStartMs() const
        {
            return viewEndMs() - static_cast<qint64>(m_windowSec) * 1000;
        }

        void QTimelineCanvas::addLogger(const LogObject::Info& info)
        {
            auto it = m_lanes.find(info.id);
            if (it == m_lanes.end())
            {
                Lane lane;
                lane.name = info.name;
                lane.color = info.color;
                lane.enabled = info.enabled;
                m_lanes.emplace(info.id, std::move(lane));
                m_laneOrder.push_back(info.id);
                setMinimumHeight(topMargin() + bottomMargin() + laneHeight() * static_cast<int>(m_laneOrder.size()));
            }
            else
            {
                it->second.name = info.name;
                it->second.color = info.color;
                it->second.enabled = info.enabled;
            }
            update();
        }
        void QTimelineCanvas::updateLogger(const LogObject::Info& info)
        {
            addLogger(info);
        }
        void QTimelineCanvas::addMessage(LoggerID id, const Message& msg)
        {
            auto it = m_lanes.find(id);
            if (it == m_lanes.end())
            {
                LogObject::Info stub;
                stub.id = id;
                stub.name = "unknown(" + std::to_string(id) + ")";
                stub.enabled = true;
                addLogger(stub);
                it = m_lanes.find(id);
            }
            if (it == m_lanes.end())
                return;
            Dot d;
            d.ms = msg.getDateTime().toQDateTime().toMSecsSinceEpoch();
            d.level = msg.getLevel();
            d.text = QString::fromStdString(msg.getText());
            it->second.dots.push_back(d);
            // Bound per-lane storage to prevent unbounded growth on long sessions.
            constexpr size_t kMaxPerLane = 20000;
            if (it->second.dots.size() > kMaxPerLane)
                it->second.dots.pop_front();
            update();
        }
        void QTimelineCanvas::clearAll()
        {
            for (auto& kv : m_lanes)
                kv.second.dots.clear();
            m_pinned.clear();
            update();
        }
        void QTimelineCanvas::setWindowSeconds(int seconds)
        {
            m_windowSec = std::max(1, seconds);
            emit zoomChanged(m_windowSec, m_followLive);
            update();
        }
        void QTimelineCanvas::setFollowLive(bool follow)
        {
            if (follow == m_followLive)
                return;
            if (!follow)
                m_frozenEndMs = QDateTime::currentMSecsSinceEpoch();
            m_followLive = follow;
            emit zoomChanged(m_windowSec, m_followLive);
            update();
        }
        void QTimelineCanvas::setLevelEnabled(Level level, bool enabled)
        {
            if (level < Level::__count)
                m_levelEnabled[level] = enabled;
            update();
        }
        void QTimelineCanvas::setTextFilter(const QString& text, bool useRegex)
        {
            QString effective = text;
            m_searchNegate = effective.startsWith('!');
            if (m_searchNegate)
                effective = effective.mid(1);
            m_searchText = effective;
            m_searchUseRegex = useRegex;
            if (useRegex && !effective.isEmpty())
            {
                m_searchRegex = QRegularExpression(effective, QRegularExpression::CaseInsensitiveOption);
                if (!m_searchRegex.isValid())
                    m_searchRegex = QRegularExpression();
            }
            else
            {
                m_searchRegex = QRegularExpression();
            }
            update();
        }
        bool QTimelineCanvas::matchesSearch(const QString& text) const
        {
            if (m_searchText.isEmpty())
                return true;
            bool hit;
            if (m_searchUseRegex)
            {
                if (!m_searchRegex.isValid() || m_searchRegex.pattern().isEmpty())
                    return true;
                hit = m_searchRegex.match(text).hasMatch();
            }
            else
            {
                hit = text.contains(m_searchText, Qt::CaseInsensitive);
            }
            return hit != m_searchNegate;
        }
        void QTimelineCanvas::setContextEnabled(LoggerID id, bool enabled)
        {
            auto it = m_lanes.find(id);
            if (it != m_lanes.end())
            {
                it->second.enabled = enabled;
                update();
            }
        }

        void QTimelineCanvas::paintEvent(QPaintEvent*)
        {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing, true);

            const int w = width();
            const int h = height();
            const int L = leftMargin();
            const int T = topMargin();
            const qint64 endMs = viewEndMs();
            const qint64 startMs = viewStartMs();
            const double spanMs = static_cast<double>(endMs - startMs);

            // Header
            p.setPen(QColor(200, 200, 200));
            const QDateTime endDt = QDateTime::fromMSecsSinceEpoch(endMs);
            const QDateTime startDt = QDateTime::fromMSecsSinceEpoch(startMs);
            p.drawText(L, 14, startDt.toString("hh:mm:ss.zzz"));
            const QString endStr = endDt.toString("hh:mm:ss.zzz");
            const int endTextW = p.fontMetrics().horizontalAdvance(endStr);
            p.drawText(w - endTextW - 4, 14, endStr);

            // Time grid (10 divisions)
            p.setPen(QColor(60, 60, 60));
            for (int i = 1; i < 10; ++i)
            {
                int x = L + (w - L) * i / 10;
                p.drawLine(x, T, x, h - bottomMargin());
            }

            // Lanes
            for (size_t i = 0; i < m_laneOrder.size(); ++i)
            {
                const auto laneIt = m_lanes.find(m_laneOrder[i]);
                if (laneIt == m_lanes.end())
                    continue;
                const Lane& lane = laneIt->second;
                const int y = T + static_cast<int>(i) * laneHeight();
                const int yc = y + laneHeight() / 2;

                // Background stripe
                if (i % 2 == 0)
                    p.fillRect(0, y, w, laneHeight(), QColor(38, 38, 38));

                // Lane name
                QColor nameColor = lane.color.toQColor();
                if (!lane.enabled)
                    nameColor = nameColor.darker(200);
                p.setPen(nameColor);
                QString name = QString::fromStdString(lane.name);
                p.drawText(4, yc + 4, name);

                // Baseline
                p.setPen(QColor(70, 70, 70));
                p.drawLine(L, yc, w - 2, yc);

                if (!lane.enabled || spanMs <= 0)
                    continue;

                // Dots: color = lane color (so each logger stays visually
                // identifiable), size + optional outline convey the level.
                const QColor laneColor = lane.color.toQColor();
                for (const auto& d : lane.dots)
                {
                    if (d.level < Level::__count && !m_levelEnabled[d.level])
                        continue;
                    if (d.ms < startMs || d.ms > endMs)
                        continue;
                    if (!matchesSearch(d.text))
                        continue;
                    const double frac = static_cast<double>(d.ms - startMs) / spanMs;
                    const int x = L + static_cast<int>(frac * (w - L - 4));
                    int r = 3;
                    switch (d.level)
                    {
                        case Level::trace:   r = 2; break;
                        case Level::debug:   r = 3; break;
                        case Level::info:    r = 4; break;
                        case Level::warning: r = 5; break;
                        case Level::error:   r = 6; break;
                        default:             r = 4; break;
                    }
                    p.setBrush(laneColor);
                    if (d.level == Level::warning || d.level == Level::error)
                    {
                        QColor ring = Message::getLevelColor(d.level).toQColor();
                        p.setPen(QPen(ring, 2));
                    }
                    else
                    {
                        p.setPen(Qt::NoPen);
                    }
                    p.drawEllipse(QPoint(x, yc), r, r);
                }
            }

            // Vertical separator between labels and timeline
            p.setPen(QColor(90, 90, 90));
            p.drawLine(L - 2, T - 4, L - 2, h - bottomMargin());

            drawHistogram(p);
            drawPinnedNotes(p);
        }

        void QTimelineCanvas::drawHistogram(QPainter& p)
        {
            if (!m_histogramVisible)
                return;
            const int w = width();
            const int h = height();
            const int L = leftMargin();
            const qint64 endMs = viewEndMs();
            const qint64 startMs = viewStartMs();
            const double spanMs = static_cast<double>(endMs - startMs);
            const int stripH = histogramHeight();
            const int stripY = h - 18 - stripH;

            p.save();
            // Backdrop for the strip.
            p.fillRect(QRect(L, stripY, w - L - 2, stripH), QColor(22, 22, 22));

            if (spanMs <= 0)
            {
                p.restore();
                return;
            }

            constexpr int N = 120; // buckets across the visible span
            std::vector<int> counts(N, 0);
            // Per-level counts so we can color the stack.
            std::vector<int> levelCounts[Level::__count];
            for (int i = 0; i < Level::__count; ++i)
                levelCounts[i].assign(N, 0);

            for (const auto& kv : m_lanes)
            {
                if (!kv.second.enabled) continue;
                for (const auto& d : kv.second.dots)
                {
                    if (d.ms < startMs || d.ms > endMs) continue;
                    if (d.level < Level::__count && !m_levelEnabled[d.level]) continue;
                    if (!matchesSearch(d.text)) continue;
                    const double frac = static_cast<double>(d.ms - startMs) / spanMs;
                    int b = static_cast<int>(frac * N);
                    if (b < 0) b = 0;
                    if (b >= N) b = N - 1;
                    ++counts[b];
                    if (d.level < Level::__count)
                        ++levelCounts[d.level][b];
                }
            }

            int maxCount = 1;
            for (int c : counts) if (c > maxCount) maxCount = c;

            const double barW = static_cast<double>(w - L - 4) / N;

            // Draw stacked bars: trace/debug/info at the base, warning/error on top.
            static const Level drawOrder[] = {
                Level::trace, Level::debug, Level::info,
                Level::custom, Level::warning, Level::error
            };
            for (int b = 0; b < N; ++b)
            {
                if (counts[b] <= 0) continue;
                const int x = L + static_cast<int>(b * barW);
                const int bw = std::max(1, static_cast<int>(barW) - 1);
                double accumFrac = 0.0;
                for (Level lv : drawOrder)
                {
                    int c = levelCounts[lv][b];
                    if (c <= 0) continue;
                    double frac = static_cast<double>(c) / maxCount;
                    int segH = static_cast<int>(frac * (stripH - 4));
                    int yTop = stripY + stripH - 2 - static_cast<int>(accumFrac * (stripH - 4)) - segH;
                    QColor col = Message::getLevelColor(lv).toQColor();
                    // Info default is white — dim it a touch so warn/err pop.
                    if (lv == Level::info) col = QColor(180, 180, 180);
                    p.fillRect(QRect(x, yTop, bw, segH), col);
                    accumFrac += frac;
                }
            }

            // Frame + max-count label.
            p.setPen(QColor(80, 80, 80));
            p.drawRect(QRect(L, stripY, w - L - 3, stripH - 1));
            p.setPen(QColor(160, 160, 160));
            const char* hint = m_histogramZoomEnabled ? "  drag to zoom · right-click to reset" : "";
            p.drawText(L + 4, stripY + 12, QString("density  max=%1/bucket%2").arg(maxCount).arg(hint));

            // Rubber-band while dragging.
            if (m_dragging)
            {
                int x1 = std::min(m_dragStartX, m_dragCurrentX);
                int x2 = std::max(m_dragStartX, m_dragCurrentX);
                QRect band(x1, stripY, x2 - x1, stripH);
                p.fillRect(band, QColor(80, 160, 240, 60));
                p.setPen(QColor(80, 160, 240));
                p.drawRect(band);
            }
            p.restore();
        }

        void QTimelineCanvas::drawPinnedNotes(QPainter& p)
        {
            const int w = width();
            const int L = leftMargin();
            const int T = topMargin();
            const qint64 endMs = viewEndMs();
            const qint64 startMs = viewStartMs();
            const double spanMs = static_cast<double>(endMs - startMs);
            if (spanMs <= 0)
                return;

            p.save();
            const int boxW = 220;
            const QFontMetrics fm(p.font());
            for (const auto& note : m_pinned)
            {
                if (note.ms < startMs || note.ms > endMs)
                    continue;
                int laneIdx = -1;
                for (size_t i = 0; i < m_laneOrder.size(); ++i)
                {
                    if (m_laneOrder[i] == note.id)
                    {
                        laneIdx = static_cast<int>(i);
                        break;
                    }
                }
                if (laneIdx < 0)
                    continue;

                const int yc = T + laneIdx * laneHeight() + laneHeight() / 2;
                const double frac = static_cast<double>(note.ms - startMs) / spanMs;
                const int x = L + static_cast<int>(frac * (w - L - 4));

                // Build note text (first line only, elided).
                QString firstLine = note.text.section('\n', 0, 0);
                firstLine = fm.elidedText(firstLine, Qt::ElideRight, boxW - 12);
                const QString ts = QDateTime::fromMSecsSinceEpoch(note.ms).toString("hh:mm:ss.zzz");
                const QString lvl = QString::fromStdString(Utilities::getLevelStr(note.level));

                const int lineH = fm.height();
                const int boxH = lineH * 2 + 8;

                // Prefer to place above the dot; fall back to below.
                int boxY = yc - laneHeight() / 2 - boxH - 6;
                bool below = false;
                if (boxY < T)
                {
                    boxY = yc + laneHeight() / 2 + 6;
                    below = true;
                }
                int boxX = x - boxW / 2;
                if (boxX < L) boxX = L;
                if (boxX + boxW > w - 2) boxX = w - 2 - boxW;

                // Lane color for the border to tie the note visually to its dot.
                QColor border(200, 200, 200);
                auto laneIt = m_lanes.find(note.id);
                if (laneIt != m_lanes.end())
                    border = laneIt->second.color.toQColor();

                // Tail from dot to box.
                p.setPen(QPen(border, 1));
                int tailY = below ? boxY : (boxY + boxH);
                p.drawLine(x, yc, x, tailY);

                // Box.
                QRect box(boxX, boxY, boxW, boxH);
                p.setBrush(QColor(20, 20, 20, 235));
                p.setPen(QPen(border, 1));
                p.drawRoundedRect(box, 4, 4);

                // Text.
                p.setPen(border);
                p.drawText(box.adjusted(6, 3, -6, -3),
                           Qt::AlignLeft | Qt::AlignTop,
                           QString("[%1] %2").arg(ts, lvl));
                p.setPen(QColor(230, 230, 230));
                p.drawText(box.adjusted(6, 3 + lineH, -6, -3),
                           Qt::AlignLeft | Qt::AlignTop, firstLine);
            }
            p.restore();
        }

        const QTimelineCanvas::Dot* QTimelineCanvas::findDotAt(const QPoint& pos, LoggerID& outId) const
        {
            const int L = leftMargin();
            const int T = topMargin();
            const qint64 endMs = viewEndMs();
            const qint64 startMs = viewStartMs();
            const double spanMs = static_cast<double>(endMs - startMs);
            if (spanMs <= 0 || pos.x() < L)
                return nullptr;
            int laneIdx = (pos.y() - T) / laneHeight();
            if (laneIdx < 0 || laneIdx >= static_cast<int>(m_laneOrder.size()))
                return nullptr;
            const int yc = T + laneIdx * laneHeight() + laneHeight() / 2;
            if (std::abs(pos.y() - yc) > laneHeight() / 2)
                return nullptr;
            const auto laneIt = m_lanes.find(m_laneOrder[laneIdx]);
            if (laneIt == m_lanes.end())
                return nullptr;
            const Lane& lane = laneIt->second;
            const int w = width();
            int bestDist = 12;
            const Dot* best = nullptr;
            for (const auto& d : lane.dots)
            {
                if (d.ms < startMs || d.ms > endMs) continue;
                if (d.level < Level::__count && !m_levelEnabled[d.level]) continue;
                if (!matchesSearch(d.text)) continue;
                const double frac = static_cast<double>(d.ms - startMs) / spanMs;
                const int x = L + static_cast<int>(frac * (w - L - 4));
                int dist = std::abs(x - pos.x());
                if (dist < bestDist)
                {
                    bestDist = dist;
                    best = &d;
                }
            }
            if (best)
                outId = m_laneOrder[laneIdx];
            return best;
        }

        bool QTimelineCanvas::event(QEvent* e)
        {
            if (e->type() == QEvent::ToolTip)
            {
                auto* he = static_cast<QHelpEvent*>(e);
                const int L = leftMargin();
                const int T = topMargin();
                const qint64 endMs = viewEndMs();
                const qint64 startMs = viewStartMs();
                const double spanMs = static_cast<double>(endMs - startMs);
                const int w = width();
                if (he->pos().x() >= L && spanMs > 0)
                {
                    int laneIdx = (he->pos().y() - T) / laneHeight();
                    if (laneIdx >= 0 && laneIdx < static_cast<int>(m_laneOrder.size()))
                    {
                        const auto laneIt = m_lanes.find(m_laneOrder[laneIdx]);
                        if (laneIt != m_lanes.end())
                        {
                            const Lane& lane = laneIt->second;
                            // Generous horizontal hit box; dot y is lane center.
                            const int yc = T + laneIdx * laneHeight() + laneHeight() / 2;
                            const int dy = std::abs(he->pos().y() - yc);
                            if (dy <= laneHeight() / 2)
                            {
                                int bestDist = 12; // px tolerance
                                const Dot* best = nullptr;
                                for (const auto& d : lane.dots)
                                {
                                    if (d.ms < startMs || d.ms > endMs) continue;
                                    if (d.level < Level::__count && !m_levelEnabled[d.level]) continue;
                                    if (!matchesSearch(d.text)) continue;
                                    const double frac = static_cast<double>(d.ms - startMs) / spanMs;
                                    const int x = L + static_cast<int>(frac * (w - L - 4));
                                    int dist = std::abs(x - he->pos().x());
                                    if (dist < bestDist)
                                    {
                                        bestDist = dist;
                                        best = &d;
                                    }
                                }
                                if (best)
                                {
                                    QString ts = QDateTime::fromMSecsSinceEpoch(best->ms).toString("hh:mm:ss.zzz");
                                    QString lvl = QString::fromStdString(Utilities::getLevelStr(best->level));
                                    QString ctx = QString::fromStdString(lane.name);
                                    QToolTip::showText(he->globalPos(),
                                        QString("<b>%1</b> &nbsp; [%2] &nbsp; <i>%3</i><br>%4")
                                            .arg(ctx, ts, lvl, best->text.toHtmlEscaped().replace("\n", "<br>")));
                                    return true;
                                }
                            }
                        }
                    }
                }
                QToolTip::hideText();
                return true;
            }
            return QWidget::event(e);
        }

        void QTimelineCanvas::wheelEvent(QWheelEvent* e)
        {
            const QPoint pos = e->position().toPoint();
            const int L = leftMargin();
            const int w = width();
            if (pos.x() < L)
            {
                QWidget::wheelEvent(e);
                return;
            }
            const int steps = e->angleDelta().y();
            if (steps == 0)
            {
                QWidget::wheelEvent(e);
                return;
            }
            // 120 units per notch. Zoom in on positive delta.
            const double factor = std::pow(1.0 / 1.2, steps / 120.0);
            const qint64 endMs = viewEndMs();
            const qint64 startMs = viewStartMs();
            const double spanMs = static_cast<double>(endMs - startMs);
            const int usable = w - L - 4;
            if (usable <= 0 || spanMs <= 0)
            {
                QWidget::wheelEvent(e);
                return;
            }
            // Anchor: keep the timestamp under the cursor in place.
            const double frac = std::max(0.0, std::min(1.0, static_cast<double>(pos.x() - L) / usable));
            const qint64 anchorMs = startMs + static_cast<qint64>(frac * spanMs);
            double newSpanMs = spanMs * factor;
            // Clamp to sane bounds: 100ms .. 24h.
            newSpanMs = std::max(100.0, std::min(24.0 * 3600.0 * 1000.0, newSpanMs));
            const qint64 newStart = anchorMs - static_cast<qint64>(frac * newSpanMs);
            const qint64 newEnd = newStart + static_cast<qint64>(newSpanMs);
            m_followLive = false;
            m_frozenEndMs = newEnd;
            m_windowSec = std::max(1, static_cast<int>(newSpanMs / 1000.0 + 0.5));
            emit zoomChanged(m_windowSec, m_followLive);
            update();
            e->accept();
        }
        QRect QTimelineCanvas::histogramStripRect() const
        {
            const int stripH = histogramHeight();
            const int stripY = height() - 18 - stripH;
            return QRect(leftMargin(), stripY, width() - leftMargin() - 2, stripH);
        }
        void QTimelineCanvas::mousePressEvent(QMouseEvent* e)
        {
            if (e->button() == Qt::LeftButton)
            {
                // Drag-to-zoom starts inside the histogram strip.
                if (m_histogramZoomEnabled && histogramStripRect().contains(e->pos()))
                {
                    m_dragging = true;
                    m_dragStartX = e->pos().x();
                    m_dragCurrentX = m_dragStartX;
                    update();
                    e->accept();
                    return;
                }
                LoggerID id = 0;
                const Dot* d = findDotAt(e->pos(), id);
                if (d)
                {
                    auto it = std::find_if(m_pinned.begin(), m_pinned.end(),
                        [&](const PinnedNote& n) { return n.id == id && n.ms == d->ms; });
                    if (it != m_pinned.end())
                        m_pinned.erase(it);
                    else
                        m_pinned.push_back({ id, d->ms, d->level, d->text });
                    update();
                    e->accept();
                    return;
                }
            }
            else if (e->button() == Qt::RightButton && !m_followLive)
            {
                m_followLive = true;
                emit zoomChanged(m_windowSec, m_followLive);
                update();
                e->accept();
                return;
            }
            QWidget::mousePressEvent(e);
        }
        void QTimelineCanvas::mouseMoveEvent(QMouseEvent* e)
        {
            if (m_dragging)
            {
                m_dragCurrentX = e->pos().x();
                update();
                e->accept();
                return;
            }
            QWidget::mouseMoveEvent(e);
        }
        void QTimelineCanvas::mouseReleaseEvent(QMouseEvent* e)
        {
            if (m_dragging && e->button() == Qt::LeftButton)
            {
                m_dragging = false;
                const QRect strip = histogramStripRect();
                int x1 = std::max(strip.left(), std::min(m_dragStartX, m_dragCurrentX));
                int x2 = std::min(strip.right(), std::max(m_dragStartX, m_dragCurrentX));
                const int minSpan = 6;
                if (x2 - x1 >= minSpan)
                {
                    const qint64 endMs = viewEndMs();
                    const qint64 startMs = viewStartMs();
                    const double spanMs = static_cast<double>(endMs - startMs);
                    const int L = leftMargin();
                    const int usable = width() - L - 4;
                    if (usable > 0 && spanMs > 0)
                    {
                        const double f1 = static_cast<double>(x1 - L) / usable;
                        const double f2 = static_cast<double>(x2 - L) / usable;
                        const qint64 t1 = startMs + static_cast<qint64>(f1 * spanMs);
                        const qint64 t2 = startMs + static_cast<qint64>(f2 * spanMs);
                        m_followLive = false;
                        m_frozenEndMs = t2;
                        int newWindow = static_cast<int>(std::max<qint64>(1, (t2 - t1) / 1000));
                        m_windowSec = std::max(1, newWindow);
                        emit zoomChanged(m_windowSec, m_followLive);
                    }
                }
                update();
                e->accept();
                return;
            }
            QWidget::mouseReleaseEvent(e);
        }

        // ----- QTimelineConsoleView -------------------------------------------

        QTimelineConsoleView::QTimelineConsoleView(QWidget* parent)
            : QAbstractLogWidget(parent)
        {
            setWindowTitle("Console timeline");

            auto* content = new QWidget();
            auto* v = new QVBoxLayout(content);
            v->setContentsMargins(4, 4, 4, 4);
            v->setSpacing(4);

            auto* header = new QWidget(content);
            auto* h = new QHBoxLayout(header);
            h->setContentsMargins(0, 0, 0, 0);
            h->addWidget(new QLabel("Window (s):", header));
            m_windowSpin = new QSpinBox(header);
            m_windowSpin->setRange(1, 3600);
            m_windowSpin->setValue(60);
            h->addWidget(m_windowSpin);
            m_followCheck = new QCheckBox("Follow live", header);
            m_followCheck->setChecked(true);
            h->addWidget(m_followCheck);
            h->addStretch(1);
            v->addWidget(header);

            m_canvas = new QTimelineCanvas(content);
            v->addWidget(m_canvas, 1);

            setContentWidget(content);

            connect(m_windowSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                    m_canvas, &QTimelineCanvas::setWindowSeconds);
            connect(m_followCheck, &QCheckBox::toggled,
                    m_canvas, &QTimelineCanvas::setFollowLive);
            connect(m_canvas, &QTimelineCanvas::zoomChanged,
                    this, [this](int windowSec, bool followLive)
                    {
                        QSignalBlocker b1(m_windowSpin);
                        QSignalBlocker b2(m_followCheck);
                        m_windowSpin->setValue(windowSec);
                        m_followCheck->setChecked(followLive);
                    });

            m_repaintTimer.setInterval(250);
            connect(&m_repaintTimer, &QTimer::timeout, m_canvas,
                    [this]() { m_canvas->update(); });
            m_repaintTimer.start();

            postConstructorInit();
        }
        QTimelineConsoleView::~QTimelineConsoleView() {}

        void QTimelineConsoleView::createStaticInstance()
        {
            QTimelineConsoleView*& p = getStaticInstance();
            if (p) return;
            p = new QTimelineConsoleView();
        }
        void QTimelineConsoleView::destroyStaticInstance()
        {
            QTimelineConsoleView*& p = getStaticInstance();
            if (p) { delete p; p = nullptr; }
        }
        QTimelineConsoleView*& QTimelineConsoleView::getStaticInstance()
        {
            static QTimelineConsoleView* p = nullptr;
            return p;
        }

        void QTimelineConsoleView::setFeatureEnabled(Feature f, bool enabled)
        {
            QAbstractLogWidget::setFeatureEnabled(f, enabled);
            switch (f)
            {
            case HistogramStrip: if (m_canvas) m_canvas->setHistogramVisible(enabled); break;
            case HistogramZoom:  if (m_canvas) m_canvas->setHistogramZoomEnabled(enabled); break;
            default: break;
            }
        }
        void QTimelineConsoleView::setDateTimeFormat(DateTime::Format format) { m_format = format; }
        DateTime::Format QTimelineConsoleView::getDateTimeFormat() const { return m_format; }

        void QTimelineConsoleView::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>&) const
        {
            // Timeline stores compacted dot data (not full Messages) — nothing to save.
        }
        void QTimelineConsoleView::clear()
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            m_canvas->clearAll();
            QAbstractLogWidget::clear();
        }

        void QTimelineConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
        {
            QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
            m_canvas->setLevelEnabled(level, isChecked);
        }
        void QTimelineConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
        {
            QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
            m_canvas->setContextEnabled(context.id, isChecked);
        }
        void QTimelineConsoleView::onDateTimeFilterChanged(const DateTimeFilter&) {}
        void QTimelineConsoleView::onSearchTextChanged(const QString& text, bool regex)
        {
            m_canvas->setTextFilter(text, regex);
        }

        void QTimelineConsoleView::onNewLogger(LogObject::Info loggerInfo)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onNewLogger(loggerInfo);
            m_canvas->addLogger(loggerInfo);
        }
        void QTimelineConsoleView::onLoggerInfoChanged(LogObject::Info info)
        {
            QAbstractLogWidget::onLoggerInfoChanged(info);
            m_canvas->updateLogger(info);
        }
        void QTimelineConsoleView::onLogMessage(Message message)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLogMessage(message);
            m_canvas->addMessage(message.getLoggerID(), message);
        }
    }
}
#endif
