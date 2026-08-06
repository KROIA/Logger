// Automated regression test for the console views' stick-to-bottom behavior.
//
// Scripted phases (mirror manual repro steps):
//   BASE  Table follows the bottom while messages stream (30ms interval).
//   A     Click "Vertical timeline" tab, click + drag the canvas, click back
//         to "Table": the table must still follow the bottom and no phantom
//         in-cell editor may have appeared (focus-transfer regression).
//   B     Wheel up on the table unsticks; the view must hold position.
//   C     End key on the scrollbar re-engages; the view follows again.
//   D     Clicking a cell opens the in-cell text-selection editor and pauses
//         following — both BY DESIGN.
//   E     Escape clears the selection and following resumes.
//   F     Tree tab: follows the bottom, wheel up unsticks, End re-engages.
//   G     Tree: clicking an item pauses following, Escape resumes it.
//
// All interaction goes through the real Qt input pipeline (QTest::mouseClick)
// where focus side effects matter; drag/wheel/key events are sent directly to
// the widgets that process them.

#include "Logger.h"
#include "ui/Widgets/QConsoleWidget.h"
#include "ui/Widgets/QContextLoggerTreeWidget.h"
#include <QApplication>
#include <QTabWidget>
#include <QTabBar>
#include <QScrollArea>
#include <QScrollBar>
#include <QTreeWidget>
#include <QTimer>
#include <QDebug>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>
#include <QtTest/QtTest>
#include <functional>
#include <vector>

using Log::UIWidgets::QConsoleWidget;
using Log::UIWidgets::QContextLoggerTreeWidget;

static QTabWidget* g_tabs = nullptr;
static QConsoleWidget* g_table = nullptr;
static QTreeWidget* g_tree = nullptr;
static QContextLoggerTreeWidget* g_treeCtl = nullptr;
static QStringList g_failures;

static bool atBottom(QAbstractScrollArea* view)
{
    QScrollBar* bar = view->verticalScrollBar();
    return bar->maximum() - bar->value() <= 1;
}

static void sample(const char* tag)
{
    QScrollBar* tb = g_table->verticalScrollBar();
    QScrollBar* rb = g_tree->verticalScrollBar();
    QWidget* fw = QApplication::focusWidget();
    qDebug().noquote()
        << tag
        << QString("tab=%1 | table v=%2/%3 sticky=%4 sel=%5 | tree v=%6/%7 sticky=%8 sel=%9 | focus=%10")
               .arg(g_tabs->currentIndex())
               .arg(tb->value()).arg(tb->maximum())
               .arg(g_table->isStickToBottom())
               .arg(g_table->hasActiveTextSelection())
               .arg(rb->value()).arg(rb->maximum())
               .arg(g_treeCtl->isStickToBottom())
               .arg(g_treeCtl->hasActiveTextSelection())
               .arg(fw ? fw->metaObject()->className() : "none");
}

static void expect(const char* tag, bool condition, const char* what)
{
    sample(tag);
    if (!condition)
        g_failures << QString("%1: %2").arg(tag, what);
}

static void wheelOn(QWidget* w, int angleDeltaY)
{
    const QPoint pos = w->rect().center();
    QWheelEvent ev(QPointF(pos), QPointF(w->mapToGlobal(pos)), QPoint(),
                   QPoint(0, angleDeltaY), Qt::NoButton, Qt::NoModifier,
                   Qt::NoScrollPhase, false);
    QApplication::sendEvent(w, &ev);
}

static void pressKey(QWidget* w, Qt::Key key)
{
    QKeyEvent kp(QEvent::KeyPress, key, Qt::NoModifier);
    QApplication::sendEvent(w, &kp);
    QKeyEvent kr(QEvent::KeyRelease, key, Qt::NoModifier);
    QApplication::sendEvent(w, &kr);
}

static void pressEscapeOnFocus(QWidget* fallback)
{
    QWidget* fw = QApplication::focusWidget();
    pressKey(fw ? fw : fallback, Qt::Key_Escape);
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Log::LogObject logger("ReproLogger");

    Log::UI::QCombinedConsoleView* combined = new Log::UI::QCombinedConsoleView();
    combined->resize(1000, 700);
    combined->show();
    combined->raise();
    combined->activateWindow();
    // Make Qt treat the window as active even if the OS keeps foreground
    // elsewhere — the focus transfer on tab switch (the original bug's
    // trigger) only runs for active windows.
    QApplication::setActiveWindow(combined);

    g_tabs = combined->findChild<QTabWidget*>();
    g_table = combined->findChild<QConsoleWidget*>();
    g_tree = combined->findChild<QTreeWidget*>();
    g_treeCtl = combined->findChild<QContextLoggerTreeWidget*>();
    if (!g_tabs || !g_table || !g_tree || !g_treeCtl)
    {
        qDebug() << "[FATAL] widgets not found" << g_tabs << g_table << g_tree << g_treeCtl;
        return 1;
    }

    // Continuous message stream.
    int msgCounter = 0;
    QTimer pump;
    QObject::connect(&pump, &QTimer::timeout, [&]() {
        logger.log("message " + std::to_string(msgCounter++), Log::Level::info);
    });
    pump.start(30);

    auto clickTab = [&](int idx) {
        QTabBar* bar = g_tabs->tabBar();
        QTest::mouseClick(bar, Qt::LeftButton, Qt::KeyboardModifiers(),
                          bar->tabRect(idx).center());
    };

    static QWidget* canvas = nullptr;
    auto findCanvas = [&]() -> QWidget* {
        const auto areas = g_tabs->widget(2)->findChildren<QScrollArea*>();
        for (QScrollArea* sa : areas)
            if (sa->widget())
                return sa->widget();
        return nullptr;
    };

    struct Step { int at; std::function<void()> fn; };
    std::vector<Step> steps = {
        { 900, [&] { expect("[BASE]", atBottom(g_table), "table not at bottom initially"); } },

        // --- A: tab round-trip with timeline interaction ---
        {1200, [&] { qDebug() << "== click timeline tab =="; clickTab(2); } },
        {1600, [&] {
            canvas = findCanvas();
            if (!canvas) { qDebug() << "[FATAL] timeline canvas not found"; return; }
            QTest::mouseClick(canvas, Qt::LeftButton, Qt::KeyboardModifiers(),
                              canvas->rect().center());
        } },
        {1900, [&] {
            if (!canvas) return;
            qDebug() << "== drag timeline ==";
            const QPoint c = canvas->rect().center();
            QMouseEvent press(QEvent::MouseButtonPress, c, canvas->mapToGlobal(c),
                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(canvas, &press);
            for (int i = 1; i <= 5; ++i)
            {
                const QPoint p = c + QPoint(0, 30 * i);
                QMouseEvent move(QEvent::MouseMove, p, canvas->mapToGlobal(p),
                                 Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(canvas, &move);
            }
            const QPoint e = c + QPoint(0, 150);
            QMouseEvent release(QEvent::MouseButtonRelease, e, canvas->mapToGlobal(e),
                                Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            QApplication::sendEvent(canvas, &release);
        } },
        {2400, [&] { qDebug() << "== click table tab =="; clickTab(0); } },
        {2700, [&] { expect("[A1]", atBottom(g_table), "table lost bottom after tab return"); } },
        {3200, [&] { expect("[A2]", atBottom(g_table) && !g_table->hasActiveTextSelection(),
                            "table lost bottom / phantom editor after tab return"); } },

        // --- B: wheel up unsticks ---
        {3500, [&] {
            qDebug() << "== wheel up on table ==";
            for (int i = 0; i < 4; ++i)
                wheelOn(g_table->viewport(), 480);
        } },
        {3800, [&] { expect("[B1]", !g_table->isStickToBottom() && !atBottom(g_table),
                            "wheel up did not unstick table"); } },
        {4100, [&] { expect("[B2]", !atBottom(g_table), "table crept back while unstuck"); } },

        // --- C: End key re-engages ---
        {4400, [&] { qDebug() << "== End key on table scrollbar ==";
                     pressKey(g_table->verticalScrollBar(), Qt::Key_End); } },
        {4700, [&] { expect("[C1]", atBottom(g_table) && g_table->isStickToBottom(),
                            "End key did not re-engage table stick"); } },
        {5100, [&] { expect("[C2]", atBottom(g_table), "table did not follow after re-engage"); } },

        // --- D: cell click pauses following (by design) ---
        {5400, [&] {
            qDebug() << "== click table cell ==";
            QTest::mouseClick(g_table->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(),
                              g_table->viewport()->rect().center());
        } },
        {5700, [&] { expect("[D1]", g_table->hasActiveTextSelection(),
                            "table click did not open in-cell editor"); } },
        {6100, [&] { expect("[D2]", !atBottom(g_table),
                            "table following not paused during selection"); } },

        // --- E: Escape clears the selection, following resumes ---
        {6400, [&] { qDebug() << "== Escape on table =="; pressEscapeOnFocus(g_table); } },
        {6700, [&] { expect("[E1]", !g_table->hasActiveTextSelection(),
                            "Escape did not clear table selection"); } },
        {7100, [&] { expect("[E2]", atBottom(g_table),
                            "table did not resume following after Escape"); } },

        // --- F: tree view stick-to-bottom ---
        {7400, [&] { qDebug() << "== click tree tab =="; clickTab(1); g_tree->expandAll(); } },
        {7800, [&] { expect("[F1]", atBottom(g_tree), "tree not at bottom after switch"); } },
        {8200, [&] { expect("[F2]", atBottom(g_tree), "tree did not follow"); } },
        {8500, [&] {
            qDebug() << "== wheel up on tree ==";
            for (int i = 0; i < 4; ++i)
                wheelOn(g_tree->viewport(), 480);
        } },
        {8800, [&] { expect("[F3]", !g_treeCtl->isStickToBottom() && !atBottom(g_tree),
                            "wheel up did not unstick tree"); } },
        {9100, [&] { expect("[F4]", !atBottom(g_tree), "tree crept back while unstuck"); } },
        {9400, [&] { qDebug() << "== End key on tree scrollbar ==";
                     pressKey(g_tree->verticalScrollBar(), Qt::Key_End); } },
        {9700, [&] { expect("[F5]", atBottom(g_tree) && g_treeCtl->isStickToBottom(),
                            "End key did not re-engage tree stick"); } },
        {10100, [&] { expect("[F6]", atBottom(g_tree), "tree did not follow after re-engage"); } },

        // --- G: tree item click pauses, Escape resumes ---
        {10400, [&] {
            qDebug() << "== click tree item ==";
            QTest::mouseClick(g_tree->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(),
                              g_tree->viewport()->rect().center());
        } },
        {10700, [&] { expect("[G1]", g_treeCtl->hasActiveTextSelection(),
                             "tree click did not open in-cell editor"); } },
        {11100, [&] { expect("[G2]", !atBottom(g_tree),
                             "tree following not paused during selection"); } },
        {11400, [&] { qDebug() << "== Escape on tree =="; pressEscapeOnFocus(g_tree); } },
        {11700, [&] { expect("[G3]", !g_treeCtl->hasActiveTextSelection(),
                             "Escape did not clear tree selection"); } },
        {12100, [&] { expect("[G4]", atBottom(g_tree),
                             "tree did not resume following after Escape"); } },

        {12500, [&] {
            sample("[FINAL]");
            if (g_failures.isEmpty())
                qDebug() << "[RESULT] PASS";
            else
            {
                qDebug() << "[RESULT] FAIL";
                for (const QString& f : g_failures)
                    qDebug().noquote() << "  -" << f;
            }
            QApplication::quit();
        } },
    };
    for (const Step& s : steps)
        QTimer::singleShot(s.at, s.fn);

    return app.exec();
}
