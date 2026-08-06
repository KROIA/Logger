#include "ui/QVerticalTimelineView.h"

#ifdef QT_WIDGETS_LIB
#include "LogMessage.h"

#include <QPainter>
#include <QDateTime>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolTip>
#include <QHelpEvent>
#include <algorithm>
#include <cmath>
#include <climits>
#include <cstdlib>
#include <functional>
#include <limits>

namespace Log
{
    namespace UI
    {
        // ---- QVerticalTimelineCanvas ----------------------------------------

        QVerticalTimelineCanvas::QVerticalTimelineCanvas(QWidget* parent)
            : QWidget(parent)
        {
            setMinimumSize(300, 200);
            setMouseTracking(true);
            setAutoFillBackground(true);
            for (int i = 0; i < Level::__count; ++i)
                m_levelEnabled[i] = true;
            // Use whatever palette the app provides so we render correctly in
            // both dark and light color schemes.

            m_leadingMs = QDateTime::currentMSecsSinceEpoch();

            // 30 fps repaint cap. We only actually paint when something
            // changed (live-tracking is on OR a message/filter/scroll happened),
            // so idle CPU stays near zero.
            m_repaintTimer.setInterval(33);
            connect(&m_repaintTimer, &QTimer::timeout, this,
                    [this]()
                    {
                        if (m_followLive || m_dirty)
                        {
                            m_dirty = false;
                            update();
                        }
                    });
            m_repaintTimer.start();
        }

        qint64 QVerticalTimelineCanvas::nowMs() const
        {
            return QDateTime::currentMSecsSinceEpoch();
        }

        int QVerticalTimelineCanvas::yFor(qint64 ms) const
        {
            const int h = height();
            const int T = contentTop();
            const int B = bottomMargin();
            const qint64 lead = m_followLive ? nowMs() : m_leadingMs;
            const double dSec = static_cast<double>(lead - ms) / 1000.0;
            if (m_direction == Direction::Down)
                return (h - B) - static_cast<int>(dSec * m_pxPerSec);
            else
                return T + static_cast<int>(dSec * m_pxPerSec);
        }

        qint64 QVerticalTimelineCanvas::msAt(int y) const
        {
            const int h = height();
            const int T = contentTop();
            const int B = bottomMargin();
            const qint64 lead = m_followLive ? nowMs() : m_leadingMs;
            if (m_direction == Direction::Down)
            {
                const double dPx = (h - B) - y;
                return lead - static_cast<qint64>(dPx / m_pxPerSec * 1000.0);
            }
            else
            {
                const double dPx = y - T;
                return lead - static_cast<qint64>(dPx / m_pxPerSec * 1000.0);
            }
        }

        void QVerticalTimelineCanvas::setDirection(Direction d)
        {
            if (d == m_direction) return;
            m_direction = d;
            // Reset to follow-live so the new leading edge is meaningful.
            m_followLive = true;
            m_leadingMs = nowMs();
            emit followLiveChanged(true);
            update();
        }

        void QVerticalTimelineCanvas::setFollowLive(bool follow)
        {
            if (follow == m_followLive) return;
            if (!follow) m_leadingMs = nowMs();
            m_followLive = follow;
            emit followLiveChanged(follow);
            update();
        }

        void QVerticalTimelineCanvas::setPixelsPerSecond(double px)
        {
            // Allow deep zoom-in so users can separate messages that arrived
            // within the same millisecond. 20000 px/s = 20 px per millisecond.
            m_pxPerSec = std::max(0.05, std::min(20000.0, px));
            update();
        }

        bool QVerticalTimelineCanvas::isHiddenByAncestorCollapse(LoggerID id) const
        {
            auto it = m_lanes.find(id);
            if (it == m_lanes.end()) return false;
            LoggerID cur = it->second.parentId;
            // Guard against pathological cycles.
            for (int i = 0; i < 1024 && cur != 0; ++i)
            {
                auto pIt = m_lanes.find(cur);
                if (pIt == m_lanes.end()) return false;
                if (pIt->second.collapsed) return true;
                cur = pIt->second.parentId;
            }
            return false;
        }

        void QVerticalTimelineCanvas::updateWidthHint()
        {
            int nVisible = 0;
            for (const auto& kv : m_lanes)
                if (kv.second.enabled && !isHiddenByAncestorCollapse(kv.first))
                    ++nVisible;
            const int desired = timeStripWidth() + std::max(1, nVisible) * minColumnWidth() + 8;
            if (minimumWidth() != desired)
                setMinimumWidth(desired);
        }
        void QVerticalTimelineCanvas::addLogger(const LogObject::Info& info)
        {
            Lane& lane = m_lanes[info.id];
            lane.name = info.name;
            lane.color = info.color;
            lane.parentId = info.parentId;
            lane.enabled = info.enabled;
            updateWidthHint();
            scheduleUpdate();
        }
        void QVerticalTimelineCanvas::updateLogger(const LogObject::Info& info)
        {
            addLogger(info);
        }
        void QVerticalTimelineCanvas::addMessage(LoggerID id, const Message& msg)
        {
            if (m_lanes.find(id) == m_lanes.end())
            {
                LogObject::Info stub;
                stub.id = id;
                stub.name = "unknown(" + std::to_string(id) + ")";
                stub.enabled = true;
                addLogger(stub);
            }
            Entry e;
            e.ms = msg.getDateTime().toQDateTime().toMSecsSinceEpoch();
            e.id = id;
            e.level = msg.getLevel();
            e.text = QString::fromStdString(msg.getText());
            m_entries.push_back(e);
            constexpr size_t kMax = 20000;
            while (m_entries.size() > kMax)
                m_entries.pop_front();
            scheduleUpdate();
        }
        void QVerticalTimelineCanvas::clearAll()
        {
            m_entries.clear();
            scheduleUpdate();
        }
        void QVerticalTimelineCanvas::setLevelEnabled(Level level, bool enabled)
        {
            if (level < Level::__count)
                m_levelEnabled[level] = enabled;
            scheduleUpdate();
        }
        void QVerticalTimelineCanvas::setContextEnabled(LoggerID id, bool enabled)
        {
            auto it = m_lanes.find(id);
            if (it != m_lanes.end())
            {
                it->second.enabled = enabled;
                updateWidthHint();
                scheduleUpdate();
            }
        }
        void QVerticalTimelineCanvas::setTextFilter(const QString& text, bool useRegex)
        {
            QString eff = text;
            m_searchNegate = eff.startsWith('!');
            if (m_searchNegate) eff = eff.mid(1);
            m_searchText = eff;
            m_searchUseRegex = useRegex;
            if (useRegex && !eff.isEmpty())
            {
                m_searchRegex = QRegularExpression(eff, QRegularExpression::CaseInsensitiveOption);
                if (!m_searchRegex.isValid())
                    m_searchRegex = QRegularExpression();
            }
            else
            {
                m_searchRegex = QRegularExpression();
            }
            scheduleUpdate();
        }
        bool QVerticalTimelineCanvas::matchesFilter(const Entry& e) const
        {
            if (e.level < Level::__count && !m_levelEnabled[e.level])
                return false;
            auto it = m_lanes.find(e.id);
            if (it != m_lanes.end() && !it->second.enabled)
                return false;
            if (!m_searchText.isEmpty())
            {
                bool hit;
                if (m_searchUseRegex)
                    hit = m_searchRegex.isValid() && m_searchRegex.match(e.text).hasMatch();
                else
                    hit = e.text.contains(m_searchText, Qt::CaseInsensitive);
                if (hit == m_searchNegate)
                    return false;
            }
            return true;
        }

        void QVerticalTimelineCanvas::paintEvent(QPaintEvent*)
        {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing, true);
            m_lastDrawn.clear();
            m_headerHits.clear();
            m_groupHits.clear();

            const int w = width();
            const int h = height();
            const int stripW = timeStripWidth();

            // Header — leading time + status
            p.setPen(palette().color(QPalette::Text));
            const qint64 lead = m_followLive ? nowMs() : m_leadingMs;
            const QString leadStr = QDateTime::fromMSecsSinceEpoch(lead).toString("hh:mm:ss.zzz");
            const QString dirStr = m_direction == Direction::Down ? "↓ newest at bottom" : "↑ newest at top";
            const QString liveStr = m_followLive ? " · live" : " · paused";
            p.drawText(4, 14, QString("Leading: %1  %2%3  ·  %4 px/s")
                       .arg(leadStr, dirStr, liveStr).arg(m_pxPerSec, 0, 'f', 1));

            // Build children map from all known lanes (parents may exist even
            // when disabled — we still need them for tree structure).
            std::unordered_map<LoggerID, std::vector<LoggerID>> children;
            std::vector<LoggerID> roots;
            for (const auto& kv : m_lanes)
            {
                const LoggerID pid = kv.second.parentId;
                if (pid != 0 && m_lanes.find(pid) != m_lanes.end())
                    children[pid].push_back(kv.first);
                else
                    roots.push_back(kv.first);
            }
            auto sortByName = [&](std::vector<LoggerID>& v)
            {
                std::sort(v.begin(), v.end(), [&](LoggerID a, LoggerID b)
                {
                    return m_lanes.at(a).name < m_lanes.at(b).name;
                });
            };
            sortByName(roots);
            for (auto& kv : children) sortByName(kv.second);

            // DFS: emit visible lanes in tree order and track per-lane depth.
            // Descendants of a collapsed lane are hidden as their own columns
            // but mapped to the collapsed ancestor for dot/bubble re-routing.
            std::vector<std::pair<LoggerID, const Lane*>> visible;
            std::vector<int> visibleDepth;
            std::unordered_map<LoggerID, LoggerID> hiddenTo; // descendant -> collapsed ancestor column
            std::unordered_map<LoggerID, int> visibleIndex;
            int maxDepth = 0;

            std::function<void(LoggerID, int, LoggerID)> dfs =
                [&](LoggerID id, int depth, LoggerID collapsedRoot)
            {
                auto it = m_lanes.find(id);
                if (it == m_lanes.end()) return;
                const Lane& lane = it->second;
                if (collapsedRoot != 0)
                {
                    hiddenTo[id] = collapsedRoot;
                }
                else if (lane.enabled)
                {
                    visibleIndex[id] = static_cast<int>(visible.size());
                    visible.push_back({ id, &lane });
                    visibleDepth.push_back(depth);
                    if (depth > maxDepth) maxDepth = depth;
                }
                const LoggerID nextCollapsed = (collapsedRoot != 0) ? collapsedRoot
                    : (lane.collapsed ? id : LoggerID(0));
                auto cIt = children.find(id);
                if (cIt != children.end())
                    for (LoggerID c : cIt->second)
                        dfs(c, depth + 1, nextCollapsed);
            };
            for (LoggerID r : roots) dfs(r, 0, 0);

            m_groupRows = maxDepth; // depths 0..maxDepth-1 need one row each

            const int T = contentTop();
            const int B = bottomMargin();

            const int nCols = static_cast<int>(visible.size());
            if (nCols == 0)
            {
                p.setPen(palette().color(QPalette::PlaceholderText));
                p.drawText(stripW + 8, T + 18, "No contexts visible — enable one in the sidebar.");
                return;
            }

            // Time window. Grid and background span the FULL widget height:
            // the leading edge stays at h - B, but dots/bubbles are allowed to
            // reach below it, so the painted background must reach the bottom
            // edge too.
            const qint64 topMs = msAt(T);
            const qint64 botMs = msAt(h);
            const qint64 viewStart = std::min(topMs, botMs);
            const qint64 viewEnd   = std::max(topMs, botMs);

            // Time-strip labels and horizontal grid lines
            if (m_pxPerSec > 0.5)
            {
                // Choose tick spacing so labels don't collide (>= 40 px apart)
                double desiredPxPerLabel = 40.0;
                double stepMs = desiredPxPerLabel / m_pxPerSec * 1000.0;
                // Snap to a nice interval
                static const double snaps[] = {1, 5, 10, 25, 50, 100, 250, 500,
                                               1000, 2000, 5000, 10000, 30000, 60000, 300000, 600000, 3600000};
                double useStep = snaps[0];
                for (double s : snaps) { if (s >= stepMs) { useStep = s; break; } }
                if (useStep < snaps[0]) useStep = snaps[0];
                qint64 stepMsInt = static_cast<qint64>(useStep);
                qint64 firstTick = (viewStart / stepMsInt) * stepMsInt;
                bool subSec = stepMsInt < 1000;

                QFont fnt = p.font();
                fnt.setPointSize(std::max(7, fnt.pointSize() - 1));
                p.setFont(fnt);
                for (qint64 t = firstTick; t <= viewEnd + stepMsInt; t += stepMsInt)
                {
                    const int y = yFor(t);
                    if (y < T - 10 || y > h + 10) continue;
                    p.setPen(palette().color(QPalette::Mid));
                    p.drawLine(stripW, y, w - 4, y);
                    // Skip the label when it would be clipped by the bottom edge.
                    if (y <= h - 4)
                    {
                        p.setPen(palette().color(QPalette::WindowText));
                        const QDateTime dt = QDateTime::fromMSecsSinceEpoch(t);
                        p.drawText(2, y + 4, subSec
                            ? dt.toString("mm:ss.zzz")
                            : dt.toString("hh:mm:ss"));
                    }
                }
                p.setFont(QApplication::font());
            }

            // Column geometry
            const int colsAreaX = stripW;
            const int colsAreaW = w - stripW - 4;
            const int colW = colsAreaW / nCols;

            // Draw column headers, tinted column backgrounds, and axis lines.
            const QFontMetrics fmHdr(p.font());
            for (int i = 0; i < nCols; ++i)
            {
                const int cx = colsAreaX + i * colW;
                const int axisX = cx + 14;
                const QColor c = visible[i].second->color.toQColor();
                const QString name = QString::fromStdString(visible[i].second->name);

                // Faint context-tinted column background. Alpha stays low so the
                // effect is legible in both dark and light color palettes; the
                // header stripe uses a slightly stronger tint.
                QColor bodyTint = c;
                bodyTint.setAlpha(22);
                p.fillRect(QRect(cx, T, colW, h - T), bodyTint);
                QColor headerTint = c;
                headerTint.setAlpha(70);
                p.fillRect(QRect(cx, topMargin(), colW, columnHeaderHeight()), headerTint);

                p.setPen(c);
                const QString ellided = fmHdr.elidedText(name, Qt::ElideRight, colW - 8);
                p.drawText(cx + 4, topMargin() + 14, ellided);

                m_headerHits.push_back({ QRect(cx, topMargin(), colW, columnHeaderHeight()),
                                          visible[i].first });

                // Divider between columns
                if (i > 0)
                {
                    p.setPen(palette().color(QPalette::Mid));
                    p.drawLine(cx, T, cx, h);
                }
                // Axis line — dim, using the palette's mid tone so it reads on
                // both dark and light backgrounds.
                p.setPen(palette().color(QPalette::Mid));
                p.drawLine(axisX, T, axisX, h);
            }

            // Group-header rows: one row per tree depth in use. Shallowest depth
            // sits at the top (breadcrumb-style). Each bar spans the visible
            // descendant columns of one ancestor at that depth.
            if (m_groupRows > 0)
            {
                // For each visible column, walk its ancestor chain to build a
                // per-(depth,ancestor) column-range and note ancestors that are
                // themselves visible columns.
                struct GroupSpan
                {
                    int minCol = std::numeric_limits<int>::max();
                    int maxCol = std::numeric_limits<int>::min();
                    bool ancestorVisible = false;
                };
                std::vector<std::unordered_map<LoggerID, GroupSpan>> byDepth(m_groupRows);
                auto extend = [](GroupSpan& s, int col)
                {
                    if (col < s.minCol) s.minCol = col;
                    if (col > s.maxCol) s.maxCol = col;
                };
                for (int i = 0; i < nCols; ++i)
                {
                    LoggerID cur = visible[i].first;
                    int d = visibleDepth[i];
                    while (d > 0)
                    {
                        auto laneIt = m_lanes.find(cur);
                        if (laneIt == m_lanes.end()) break;
                        LoggerID pid = laneIt->second.parentId;
                        if (pid == 0 || m_lanes.find(pid) == m_lanes.end()) break;
                        --d;
                        extend(byDepth[d][pid], i);
                        cur = pid;
                    }
                }
                // If the ancestor is itself a visible column, include its own column.
                for (int d = 0; d < m_groupRows; ++d)
                {
                    for (auto& kv : byDepth[d])
                    {
                        auto vIt = visibleIndex.find(kv.first);
                        if (vIt != visibleIndex.end())
                        {
                            extend(kv.second, vIt->second);
                            kv.second.ancestorVisible = true;
                        }
                    }
                }

                const int colsAreaX0 = stripW;
                const QFontMetrics fmG(p.font());
                for (int d = 0; d < m_groupRows; ++d)
                {
                    const int y = baseTopMargin() + d * groupRowHeight();
                    for (const auto& kv : byDepth[d])
                    {
                        const LoggerID aid = kv.first;
                        const GroupSpan& s = kv.second;
                        // Skip degenerate single-column groups where the ancestor
                        // isn't itself visible — avoids noise for single-child chains.
                        if (s.minCol == s.maxCol && !s.ancestorVisible) continue;

                        auto aIt = m_lanes.find(aid);
                        if (aIt == m_lanes.end()) continue;
                        const Lane& aLane = aIt->second;
                        const int x0 = colsAreaX0 + s.minCol * colW;
                        const int x1 = colsAreaX0 + (s.maxCol + 1) * colW;
                        const QRect bar(x0, y, x1 - x0, groupRowHeight() - 1);
                        QColor tint = aLane.color.toQColor();
                        tint.setAlpha(90);
                        p.fillRect(bar, tint);
                        p.setPen(palette().color(QPalette::Mid));
                        p.drawRect(bar);

                        // [-]/[+] glyph on the left.
                        const QRect glyph(bar.left() + 2, bar.top() + 1,
                                          groupRowHeight() - 2, groupRowHeight() - 2);
                        p.setPen(palette().color(QPalette::WindowText));
                        p.drawText(glyph, Qt::AlignCenter, aLane.collapsed ? "+" : "-");
                        m_groupHits.push_back({ glyph, aid });

                        // Ancestor name, elided.
                        p.setPen(aLane.color.toQColor());
                        const int textX = glyph.right() + 4;
                        const int textW = bar.right() - textX - 2;
                        if (textW > 8)
                        {
                            const QString nm = QString::fromStdString(aLane.name);
                            p.drawText(QRect(textX, bar.top(), textW, bar.height()),
                                       Qt::AlignVCenter | Qt::AlignLeft,
                                       fmG.elidedText(nm, Qt::ElideRight, textW));
                        }
                    }
                }
            }

            const QFontMetrics fm(p.font());
            const int lineH = fm.height();
            const int connectorGap = 28; // space between axis dot and bubble left edge
            const bool bubblesOn = m_pxPerSec >= 3.0 && (colW - (14 + connectorGap + 12)) >= 60;

            // m_entries is already in chronological order (messages arrive
            // monotonically). Iterate forward for Down, reverse for Up —
            // no per-frame sort needed.

            // Per-column state
            std::unordered_map<LoggerID, int> colIndexById;
            for (int i = 0; i < nCols; ++i) colIndexById[visible[i].first] = i;
            std::vector<int> prevBottom(nCols, std::numeric_limits<int>::min());

            auto processEntry = [&](const Entry& e)
            {
                if (!matchesFilter(e))
                    return;
                bool fromDescendant = false;
                auto colIt = colIndexById.find(e.id);
                if (colIt == colIndexById.end())
                {
                    // Entry's own lane isn't a visible column — see if it was
                    // routed to a collapsed ancestor.
                    auto hIt = hiddenTo.find(e.id);
                    if (hIt == hiddenTo.end()) return;
                    colIt = colIndexById.find(hIt->second);
                    if (colIt == colIndexById.end()) return;
                    fromDescendant = true;
                }
                const int col = colIt->second;
                const int cx = colsAreaX + col * colW;
                const int axisX = cx + 14;
                // Outer ring keeps the true source-context color (already the
                // design); when routed from a descendant we still use the
                // originating lane's color for the ring so the source is legible.
                QColor laneColor = visible[col].second->color.toQColor();
                if (fromDescendant)
                {
                    auto srcIt = m_lanes.find(e.id);
                    if (srcIt != m_lanes.end())
                        laneColor = srcIt->second.color.toQColor();
                }
                const int naturalY = yFor(e.ms);

                // Skip entries above the visible viewport so their off-screen
                // bubbles don't push visible ones out of place. The first entry
                // that reaches the top of the viewport anchors the cascade at
                // its natural Y — subsequent entries below stack from there.
                if (naturalY < T - 2)
                    return;
                // Same for entries whose natural position is far below the
                // viewport: they can't be visible, and since we iterate in
                // display order they can't affect earlier bubbles either.
                if (naturalY > h - B + 40)
                    return;

                // Dot at natural Y — fill = level color (severity), outer ring = context color (identity)
                int r = 4;
                switch (e.level)
                {
                case Level::trace:   r = 3; break;
                case Level::debug:   r = 4; break;
                case Level::info:    r = 5; break;
                case Level::warning: r = 6; break;
                case Level::error:   r = 7; break;
                default:             r = 5; break;
                }
                const bool dotInView = naturalY >= T - 20 && naturalY <= h - B + 20;
                if (dotInView)
                {
                    p.setBrush(Message::getLevelColor(e.level).toQColor());
                    p.setPen(QPen(laneColor, 2));
                    p.drawEllipse(QPoint(axisX, naturalY), r, r);
                    if (fromDescendant)
                    {
                        p.setPen(laneColor);
                        p.drawText(QPoint(axisX + r + 2, naturalY + 4), QStringLiteral("›"));
                    }
                }

                if (!bubblesOn)
                    return;

                const int textX = axisX + connectorGap;
                const int bubbleW = cx + colW - textX - 4;
                if (bubbleW < 60)
                    return;

                // Bubble height: measure once via word-wrap layout and cache
                // per (text, width). Word-wrap is the single biggest per-frame
                // cost, so this cache is what makes 30 fps feasible.
                const int textInnerW = bubbleW - 12;
                const int textFlags = Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop;
                int bubbleH;
                if (e.cachedBubbleW == textInnerW && e.cachedBubbleH > 0)
                {
                    bubbleH = e.cachedBubbleH;
                }
                else
                {
                    const QRect measureRect(0, 0, textInnerW, 100000);
                    const QRect wrapped = fm.boundingRect(measureRect, textFlags, e.text);
                    const int bodyH = std::max(lineH, wrapped.height());
                    bubbleH = lineH + bodyH + 10;
                    e.cachedBubbleW = textInnerW;
                    e.cachedBubbleH = bubbleH;
                }

                // Place bubble: prefer natural Y, shift past previous bubble in this column.
                int by = naturalY - bubbleH / 2;
                if (by < prevBottom[col] + 2)
                    by = prevBottom[col] + 2;
                prevBottom[col] = by + bubbleH;

                if (by + bubbleH < T - 40 || by > h - B + 40)
                    return;

                // Bubble accent color = level color. Fill = a desaturated tint
                // whose lightness follows the active palette (dark tint on dark
                // themes, light tint on light themes) so text stays readable.
                const bool lightMode = palette().color(QPalette::Base).lightness() > 128;
                const QColor lvlRaw = Message::getLevelColor(e.level).toQColor();
                int hh, ss, ll;
                lvlRaw.getHsl(&hh, &ss, &ll);
                if (hh < 0) hh = 0; // achromatic (e.g. pure white/gray)
                QColor bubbleFill = QColor::fromHsl(hh, ss / 3, lightMode ? 220 : 60);
                // Border / connector / header text: use the raw level color in
                // dark mode, a darkened version in light mode (so near-white
                // level colors don't disappear against a light bubble fill).
                const QColor lvlColor = lightMode ? lvlRaw.darker(180) : lvlRaw;

                // Connector from dot to bubble edge.
                p.setPen(QPen(lvlColor, 2));
                const int bubbleCenterY = by + bubbleH / 2;
                p.drawLine(axisX + r, naturalY, textX, bubbleCenterY);

                const QRect bubble(textX, by, bubbleW, bubbleH);
                p.setBrush(bubbleFill);
                p.setPen(QPen(lvlColor, 1));
                p.drawRoundedRect(bubble, 4, 4);

                p.setPen(lvlColor);
                const QString ts  = QDateTime::fromMSecsSinceEpoch(e.ms).toString("hh:mm:ss.zzz");
                const QString lvl = QString::fromStdString(Utilities::getLevelStr(e.level));
                p.drawText(bubble.adjusted(6, 3, -6, -3),
                           Qt::AlignLeft | Qt::AlignTop,
                           QString("[%1] %2").arg(ts, lvl));

                // Body text color follows the theme so it contrasts against
                // the mode-dependent bubble fill.
                p.setPen(lightMode ? QColor(30, 30, 30) : QColor(235, 235, 235));
                const QRect textRect = bubble.adjusted(6, 3 + lineH, -6, -3);
                p.drawText(textRect, textFlags, e.text);

                DrawnBubble db;
                db.rect = bubble;
                db.dotY = naturalY;
                db.axisX = axisX;
                db.id = e.id;
                db.ms = e.ms;
                db.level = e.level;
                db.text = e.text;
                m_lastDrawn.push_back(db);
            };

            if (m_direction == Direction::Down)
            {
                for (const auto& e : m_entries)
                    processEntry(e);
            }
            else
            {
                for (auto it = m_entries.rbegin(); it != m_entries.rend(); ++it)
                    processEntry(*it);
            }

            if (!bubblesOn)
            {
                p.setPen(palette().color(QPalette::PlaceholderText));
                p.drawText(stripW + 8, h - B - 6, "message bubbles hidden — zoom in to reveal");
            }
        }

        void QVerticalTimelineCanvas::wheelEvent(QWheelEvent* e)
        {
            const int steps = e->angleDelta().y();
            if (steps == 0) { QWidget::wheelEvent(e); return; }

            // Shift+wheel scrolls the parent QScrollArea horizontally so the
            // user can pan across many context columns without dragging.
            if (e->modifiers() & Qt::ShiftModifier)
            {
                QWidget* w = parentWidget();
                while (w)
                {
                    if (auto* sa = qobject_cast<QScrollArea*>(w))
                    {
                        auto* bar = sa->horizontalScrollBar();
                        if (bar)
                        {
                            const int step = std::max(bar->singleStep(), 24);
                            bar->setValue(bar->value() - (steps / 120) * step);
                            e->accept();
                            return;
                        }
                        break;
                    }
                    w = w->parentWidget();
                }
                e->ignore();
                return;
            }

            const double factor = std::pow(1.2, steps / 120.0);
            const QPoint pos = e->position().toPoint();
            // Anchor: keep the ms under the cursor pinned.
            const qint64 anchorMs = msAt(pos.y());
            setPixelsPerSecond(m_pxPerSec * factor);
            // After zoom, adjust leadingMs so anchor stays at same y.
            // Only meaningful when paused; when live, follow keeps overriding.
            if (!m_followLive)
            {
                const qint64 curYms = msAt(pos.y());
                const qint64 delta = anchorMs - curYms;
                m_leadingMs += delta;
                maybeReenableFollowLive();
            }
            update();
            e->accept();
        }

        void QVerticalTimelineCanvas::mousePressEvent(QMouseEvent* e)
        {
            if (e->button() == Qt::LeftButton)
            {
                // Group-bar [+]/[-] glyph or per-context header toggles collapse.
                for (const auto& g : m_groupHits)
                {
                    if (g.glyphRect.contains(e->pos()))
                    {
                        auto it = m_lanes.find(g.id);
                        if (it != m_lanes.end())
                        {
                            it->second.collapsed = !it->second.collapsed;
                            updateWidthHint();
                            scheduleUpdate();
                            update();
                        }
                        e->accept();
                        return;
                    }
                }
                for (const auto& hh : m_headerHits)
                {
                    if (hh.rect.contains(e->pos()))
                    {
                        auto it = m_lanes.find(hh.id);
                        if (it != m_lanes.end())
                        {
                            it->second.collapsed = !it->second.collapsed;
                            updateWidthHint();
                            scheduleUpdate();
                            update();
                        }
                        e->accept();
                        return;
                    }
                }
                m_dragging = true;
                m_dragStartY = e->pos().y();
                if (m_followLive)
                {
                    m_followLive = false;
                    m_leadingMs = nowMs();
                    emit followLiveChanged(false);
                }
                m_dragStartLeading = m_leadingMs;
                e->accept();
                return;
            }
            if (e->button() == Qt::RightButton)
            {
                // Right-click resumes live.
                if (!m_followLive)
                {
                    m_followLive = true;
                    emit followLiveChanged(true);
                    update();
                }
                e->accept();
                return;
            }
            QWidget::mousePressEvent(e);
        }
        void QVerticalTimelineCanvas::mouseMoveEvent(QMouseEvent* e)
        {
            if (m_dragging)
            {
                const int dy = e->pos().y() - m_dragStartY;
                const qint64 dMs = static_cast<qint64>(dy / m_pxPerSec * 1000.0);
                if (m_direction == Direction::Down)
                    m_leadingMs = m_dragStartLeading - dMs;
                else
                    m_leadingMs = m_dragStartLeading + dMs;
                update();
                e->accept();
                return;
            }
            QWidget::mouseMoveEvent(e);
        }
        void QVerticalTimelineCanvas::mouseReleaseEvent(QMouseEvent* e)
        {
            if (m_dragging && e->button() == Qt::LeftButton)
            {
                m_dragging = false;
                maybeReenableFollowLive();
                e->accept();
                return;
            }
            QWidget::mouseReleaseEvent(e);
        }
        void QVerticalTimelineCanvas::mouseDoubleClickEvent(QMouseEvent* e)
        {
            if (e->button() == Qt::LeftButton)
            {
                for (const auto& db : m_lastDrawn)
                {
                    if (db.rect.contains(e->pos()))
                    {
                        QApplication::clipboard()->setText(db.text);
                        e->accept();
                        return;
                    }
                }
            }
            QWidget::mouseDoubleClickEvent(e);
        }
        void QVerticalTimelineCanvas::maybeReenableFollowLive()
        {
            if (m_followLive)
                return;
            // Pixel-based snap: check whether "now" would render at (or past)
            // the leading edge of the viewport. Using pixels scales with zoom —
            // a fixed ms tolerance was too loose zoomed in, too tight zoomed out.
            const int leadingY = (m_direction == Direction::Down)
                ? (height() - bottomMargin())
                : contentTop();
            const int yOfNow = yFor(nowMs());
            const int distFromLeadingPx = (m_direction == Direction::Down)
                ? (yOfNow - leadingY)   // > 0 means "now" is past the leading edge (time
                : (leadingY - yOfNow);  // advanced while paused — user has NOT caught up).
            // Only snap to live when the user has actually scrolled to the leading edge:
            // m_leadingMs must be at or ahead of nowMs (dist <= small tolerance). A positive
            // distance means the paused view is trailing real-time and should stay paused.
            if (distFromLeadingPx <= 6)
            {
                m_followLive = true;
                m_leadingMs = nowMs();
                emit followLiveChanged(true);
                scheduleUpdate();
            }
        }

        bool QVerticalTimelineCanvas::event(QEvent* e)
        {
            if (e->type() == QEvent::ToolTip)
            {
                auto* he = static_cast<QHelpEvent*>(e);
                const QPoint pt = he->pos();
                const DrawnBubble* best = nullptr;
                // Prefer a bubble whose rect contains the mouse.
                for (const auto& db : m_lastDrawn)
                {
                    if (db.rect.contains(pt)) { best = &db; break; }
                }
                // Otherwise nearest dot by y (within its column's x range).
                if (!best)
                {
                    int bestDist = 16;
                    for (const auto& db : m_lastDrawn)
                    {
                        if (std::abs(db.axisX - pt.x()) > 16) continue;
                        int d = std::abs(db.dotY - pt.y());
                        if (d < bestDist)
                        {
                            bestDist = d;
                            best = &db;
                        }
                    }
                }
                if (best)
                {
                    const QString ts = QDateTime::fromMSecsSinceEpoch(best->ms).toString("hh:mm:ss.zzz");
                    const QString lvl = QString::fromStdString(Utilities::getLevelStr(best->level));
                    auto laneIt = m_lanes.find(best->id);
                    const QString ctx = laneIt != m_lanes.end()
                        ? QString::fromStdString(laneIt->second.name) : QString();
                    QToolTip::showText(he->globalPos(),
                        QString("<b>%1</b> &nbsp; [%2] &nbsp; <i>%3</i><br>%4")
                            .arg(ctx.toHtmlEscaped(), ts, lvl,
                                 best->text.toHtmlEscaped().replace("\n", "<br>")));
                    return true;
                }
                QToolTip::hideText();
                return true;
            }
            return QWidget::event(e);
        }

        // ---- QVerticalTimelineView ------------------------------------------

        QVerticalTimelineView::QVerticalTimelineView(QWidget* parent)
            : QAbstractLogWidget(parent)
        {
            setWindowTitle("Console timeline (vertical)");
            m_canvas = new QVerticalTimelineCanvas();
            // Wrap the canvas in a horizontal scroll area so many contexts stay
            // legible instead of being squeezed into a fixed viewport.
            auto* scroll = new QScrollArea();
            scroll->setWidget(m_canvas);
            scroll->setWidgetResizable(true);
            scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            scroll->setFrameShape(QFrame::NoFrame);
            setContentWidget(scroll);
            postConstructorInit();
        }
        QVerticalTimelineView::~QVerticalTimelineView() {}

        void QVerticalTimelineView::createStaticInstance()
        {
            QVerticalTimelineView*& p = getStaticInstance();
            if (p) return;
            p = new QVerticalTimelineView();
        }
        void QVerticalTimelineView::destroyStaticInstance()
        {
            QVerticalTimelineView*& p = getStaticInstance();
            if (p) { delete p; p = nullptr; }
        }
        QVerticalTimelineView*& QVerticalTimelineView::getStaticInstance()
        {
            static QVerticalTimelineView* p = nullptr;
            return p;
        }

        void QVerticalTimelineView::setDateTimeFormat(DateTime::Format format) { m_format = format; }
        DateTime::Format QVerticalTimelineView::getDateTimeFormat() const { return m_format; }

        void QVerticalTimelineView::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>&) const
        {
            // Vertical timeline holds compacted entries, not full Messages.
        }
        void QVerticalTimelineView::clear()
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            m_canvas->clearAll();
            QAbstractLogWidget::clear();
        }
        void QVerticalTimelineView::setDirection(Direction d) { m_canvas->setDirection(d); }
        QVerticalTimelineView::Direction QVerticalTimelineView::direction() const { return m_canvas->direction(); }

        void QVerticalTimelineView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
        {
            QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
            m_canvas->setLevelEnabled(level, isChecked);
        }
        void QVerticalTimelineView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
        {
            QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
            m_canvas->setContextEnabled(context.id, isChecked);
        }
        void QVerticalTimelineView::onDateTimeFilterChanged(const DateTimeFilter&) {}
        void QVerticalTimelineView::onSearchTextChanged(const QString& text, bool regex)
        {
            m_canvas->setTextFilter(text, regex);
        }
        void QVerticalTimelineView::onNewLogger(LogObject::Info loggerInfo)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onNewLogger(loggerInfo);
            m_canvas->addLogger(loggerInfo);
        }
        void QVerticalTimelineView::onLoggerInfoChanged(LogObject::Info info)
        {
            QAbstractLogWidget::onLoggerInfoChanged(info);
            m_canvas->updateLogger(info);
        }
        void QVerticalTimelineView::onLogMessage(Message message)
        {
            LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
            QAbstractLogWidget::onLogMessage(message);
            m_canvas->addMessage(message.getLoggerID(), message);
        }
    }
}
#endif
