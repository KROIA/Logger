#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include "ui/Widgets/QAbstractLogWidget.h"
#include <QWidget>
#include <QTimer>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QRegularExpression>
#include <deque>
#include <unordered_map>
#include <vector>

namespace Log
{
    namespace UI
    {
        // Internal renderer used by QTimelineConsoleView. One horizontal
        // swimlane per logger context, colored dots along the x-axis at each
        // message's timestamp. Kept in the same TU as the view — this widget
        // has no purpose outside it.
        class QTimelineCanvas : public QWidget
        {
            Q_OBJECT
        public:
            struct Dot
            {
                qint64 ms;
                Level level;
                QString text;
            };
            struct Lane
            {
                std::string name;
                Color color;
                bool enabled = true;
                std::deque<Dot> dots;
            };

            QTimelineCanvas(QWidget* parent = nullptr);

        signals:
            void zoomChanged(int windowSec, bool followLive);
        public:

            void addMessage(LoggerID id, const Message& msg);
            void addLogger(const LogObject::Info& info);
            void updateLogger(const LogObject::Info& info);
            void clearAll();

            void setWindowSeconds(int seconds);
            void setFollowLive(bool follow);
            void setLevelEnabled(Level level, bool enabled);
            void setContextEnabled(LoggerID id, bool enabled);
            void setTextFilter(const QString& text, bool useRegex);

        protected:
            void paintEvent(QPaintEvent*) override;
            bool event(QEvent* e) override;
            void mousePressEvent(QMouseEvent* e) override;
            void mouseMoveEvent(QMouseEvent* e) override;
            void mouseReleaseEvent(QMouseEvent* e) override;
            void wheelEvent(QWheelEvent* e) override;

        private:
            struct PinnedNote
            {
                LoggerID id;
                qint64 ms;
                Level level;
                QString text;
            };
            const Dot* findDotAt(const QPoint& pos, LoggerID& outId) const;
            void drawPinnedNotes(QPainter& p);
            void drawHistogram(QPainter& p);
            std::vector<PinnedNote> m_pinned;

        private:
            int laneHeight() const { return 22; }
            int leftMargin() const { return m_leftMargin; }
            int topMargin() const { return 22; }
            int histogramHeight() const { return m_histogramVisible ? 44 : 0; }
            int bottomMargin() const { return 18 + histogramHeight(); }
            qint64 viewStartMs() const;
            qint64 viewEndMs() const;

            int m_leftMargin = 120;
            int m_windowSec = 60;
            bool m_followLive = true;
            qint64 m_frozenEndMs = 0;

            // Drag-to-zoom on the histogram strip.
            bool m_histogramZoomEnabled = true;
            bool m_dragging = false;
            int m_dragStartX = 0;
            int m_dragCurrentX = 0;
        public:
            void setHistogramZoomEnabled(bool enabled) { m_histogramZoomEnabled = enabled; }
            void setHistogramVisible(bool visible) { m_histogramVisible = visible; update(); }
        private:
            bool m_histogramVisible = true;
            QRect histogramStripRect() const;

            bool m_levelEnabled[Level::__count];
            QString m_searchText;
            bool m_searchUseRegex = false;
            bool m_searchNegate = false;
            QRegularExpression m_searchRegex;
            bool matchesSearch(const QString& text) const;

            std::unordered_map<LoggerID, Lane> m_lanes;
            std::vector<LoggerID> m_laneOrder;
        };

        // Swimlane timeline view. Each logger occupies one lane; each message
        // is drawn as a colored dot at its timestamp. Complements table/tree
        // views by making temporal density and cross-context correlation
        // visible at a glance.
        class LOGGER_API QTimelineConsoleView : public UIWidgets::QAbstractLogWidget
        {
            Q_OBJECT
        public:
            QTimelineConsoleView(QWidget* parent = nullptr);
            ~QTimelineConsoleView();

            static void createStaticInstance();
            static void destroyStaticInstance();
            static QTimelineConsoleView*& getStaticInstance();

            void setDateTimeFormat(DateTime::Format format) override;
            DateTime::Format getDateTimeFormat() const override;
            void setFeatureEnabled(Feature f, bool enabled) override;

            void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const override;
            void clear() override;

        private:
            void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked) override;
            void onContextCheckBoxChanged(const ContextData& context, bool isChecked) override;
            void onDateTimeFilterChanged(const DateTimeFilter& filter) override;

            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLoggerInfoChanged(LogObject::Info info) override;
            void onLogMessage(Message message) override;
            void onSearchTextChanged(const QString& text, bool regex) override;

            DateTime::Format m_format = DateTime::Format::hourMinuteSecondMillisecond;
            QTimelineCanvas* m_canvas = nullptr;
            QSpinBox* m_windowSpin = nullptr;
            QCheckBox* m_followCheck = nullptr;
            QTimer m_repaintTimer;
        };
    }
}
#endif
