#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "ui/Widgets/QAbstractLogWidget.h"
#include <QWidget>
#include <QTimer>
#include <QRegularExpression>
#include <deque>
#include <unordered_map>

namespace Log
{
    namespace UI
    {
        // Canvas widget that draws the vertical timeline. Kept in the same TU
        // as the view since it has no purpose outside it.
        class QVerticalTimelineCanvas : public QWidget
        {
            Q_OBJECT
        public:
            enum class Direction
            {
                Down,   // newer messages appear at the bottom; auto-scroll clamps to bottom
                Up      // newer messages appear at the top; auto-scroll clamps to top
            };

            // Present: axis anchored to wall-clock "now" and auto-follows live
            // data. Past: axis anchored to loaded data (fit to its time span),
            // auto-follow permanently disabled — used for file-loaded logs
            // whose timestamps lie in the past.
            enum class Mode
            {
                Present,
                Past
            };

            struct Entry
            {
                qint64 ms;
                LoggerID id;
                Level level;
                QString text;
                // Cached bubble height for the last-known bubble width. Avoids
                // re-running fm.boundingRect() (word-wrap layout) every frame
                // for entries whose text/width didn't change.
                mutable int cachedBubbleW = -1;
                mutable int cachedBubbleH = -1;
            };
            struct Lane
            {
                std::string name;
                Color color;
                LoggerID parentId = 0;
                bool enabled = true;
                bool collapsed = false;
            };

            QVerticalTimelineCanvas(QWidget* parent = nullptr);

            void addMessage(LoggerID id, const Message& msg);
            void addLogger(const LogObject::Info& info);
            void updateLogger(const LogObject::Info& info);
            void clearAll();

            void setLevelEnabled(Level level, bool enabled);
            void setContextEnabled(LoggerID id, bool enabled);
            void setTextFilter(const QString& text, bool useRegex);

            void setDirection(Direction d);
            Direction direction() const { return m_direction; }

            void setFollowLive(bool follow);
            bool followLive() const { return m_followLive; }

            void setMode(Mode m);
            Mode mode() const { return m_mode; }

            void setPixelsPerSecond(double px);
            double pixelsPerSecond() const { return m_pxPerSec; }

        signals:
            void followLiveChanged(bool followLive);

        protected:
            void paintEvent(QPaintEvent*) override;
            void wheelEvent(QWheelEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseDoubleClickEvent(QMouseEvent*) override;
            bool event(QEvent*) override;

        private:
            int timeStripWidth() const { return 62; }
            int baseTopMargin() const { return 22; }
            int groupRowHeight() const { return 16; }
            int topMargin() const { return baseTopMargin() + m_groupRows * groupRowHeight(); }
            int columnHeaderHeight() const { return 20; }
            int contentTop() const { return topMargin() + columnHeaderHeight(); }
            int bottomMargin() const { return 18; }
            int minColumnWidth() const { return 320; }
            qint64 nowMs() const;
            int yFor(qint64 ms) const;
            qint64 msAt(int y) const;
            void maybeReenableFollowLive();
            // Scan m_entries for their min/max timestamp and anchor the axis so
            // the whole span fits the drawable height. Used by Past mode.
            void fitToData();
            bool matchesFilter(const Entry& e) const;
            void updateWidthHint();
            bool isHiddenByAncestorCollapse(LoggerID id) const;

            struct DrawnBubble
            {
                QRect rect;
                int dotY;
                int axisX;
                LoggerID id;
                qint64 ms;
                Level level;
                QString text;
            };
            mutable std::vector<DrawnBubble> m_lastDrawn;

            struct HeaderHit { QRect rect; LoggerID id; };
            struct GroupHit  { QRect glyphRect; LoggerID id; };
            std::vector<HeaderHit> m_headerHits;
            std::vector<GroupHit>  m_groupHits;
            int m_groupRows = 0;

            std::deque<Entry> m_entries;
            std::unordered_map<LoggerID, Lane> m_lanes;
            bool m_levelEnabled[Level::__count];

            QString m_searchText;
            bool m_searchUseRegex = false;
            bool m_searchNegate = false;
            QRegularExpression m_searchRegex;

            Direction m_direction = Direction::Down;
            Mode m_mode = Mode::Present;
            bool m_followLive = true;
            qint64 m_leadingMs = 0;   // time at the leading edge (bottom for Down, top for Up)
            double m_pxPerSec = 20.0;

            bool m_dragging = false;
            int m_dragStartY = 0;
            qint64 m_dragStartLeading = 0;

            QTimer m_repaintTimer;
            bool m_dirty = false;      // "please repaint next tick"
            void scheduleUpdate() { m_dirty = true; }
        };

        // Vertical-timeline console view: dots + always-visible message bubbles
        // arranged along a vertical time axis. Complements the horizontal
        // timeline by showing full message text at every position.
        class LOGGER_API QVerticalTimelineView : public UIWidgets::QAbstractLogWidget
        {
            Q_OBJECT
        public:
            using Direction = QVerticalTimelineCanvas::Direction;
            using Mode = QVerticalTimelineCanvas::Mode;

            QVerticalTimelineView(QWidget* parent = nullptr);
            ~QVerticalTimelineView();

            static void createStaticInstance();
            static void destroyStaticInstance();
            static QVerticalTimelineView*& getStaticInstance();

            void setDateTimeFormat(DateTime::Format format) override;
            DateTime::Format getDateTimeFormat() const override;

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const override;
            void clear() override;

            // Programmatic direction control. Default is Down (newer at bottom).
            void setDirection(Direction d);
            Direction direction() const;

            // Present (live, wall-clock anchored) vs Past (fit to loaded data).
            void setMode(Mode m);
            Mode mode() const;

            QVerticalTimelineCanvas* canvas() const { return m_canvas; }

        private:
            void onMessagesLoaded() override;
            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(const ContextData& context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;
            void onSearchTextChanged(const QString& text, bool regex) override;

            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLoggerInfoChanged(LogObject::Info info) override;
            void onLogMessage(Message message) override;

            DateTime::Format m_format = DateTime::Format::hourMinuteSecondMillisecond;
            QVerticalTimelineCanvas* m_canvas = nullptr;
        };
    }
}
#endif
