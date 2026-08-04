# Logger

## Overview
* [About](#about)
* [Concept](#concept)
* [Installation](#installation)
  * [Dependencies](#dependencies)
  * [How to build](#how-to-build)
* [How to use](#how-to-use)
  * [Basic example](#basic-example)
  * [Nested loggers](#nested-loggers)
  * [Custom colors](#custom-colors)
* [Receiver types](#receiver-types)
  * [QConsoleView](#qconsoleview)
  * [QTreeConsoleView](#qtreeconsoleview)
  * [QCombinedConsoleView](#qcombinedconsoleview)
  * [QTimelineConsoleView](#qtimelineconsoleview)
  * [QStatsConsoleView](#qstatsconsoleview)
  * [NativeConsoleView](#nativeconsoleview)
  * [FilePlotter](#fileplotter)
  * [Custom receiver implementation](#custom-receiver-implementation)
* [View features](#view-features)
  * [Search bar](#search-bar)
  * [Find next / previous](#find-next--previous)
  * [Level and context filters](#level-and-context-filters)
  * [Date-time filter](#date-time-filter)
  * [Details pane](#details-pane)
  * [Row context menu](#row-context-menu)
  * [In-cell text selection and copy](#in-cell-text-selection-and-copy)
  * [Feature flags](#feature-flags)
  * [Sub-widget toggles](#sub-widget-toggles)
* [Persisting logs](#persisting-logs)
  * [Saving visible messages](#saving-visible-messages)
  * [Loading a log file](#loading-a-log-file)
  * [File format](#file-format)


## About
> <img src="documentation/Images/bookshelf.png" alt="QT_cmake_library_template"  width="40" style="vertical-align:middle;"> This library was created using the [library template](https://github.com/KROIA/QT_cmake_library_template)


The logger library can be used in your project to print messages to the console. A message is categorized in 6 levels:

|Level |Meaning |
|---|---|
|<span style="color:rgb(0,255,255)">trace</span>|Can be used to print call stacks|
|<span style="color:rgb(255,0,255)">debug</span>|Debug infos that are used for development|
|<span style="color:rgb(0,0,0)">info</span>|Info outputs that can be helpfull for the end user|
|<span style="color:rgb(230,230,0)">warning</span>|Warning messages|
|<span style="color:rgb(255,0,0)">error</span>|Error messages|
|<span style="color:rgb(0,255,0)">custom</span>|No specific usecase, just another separate category |

---
## Concept
The library uses the Qt signal & slot system to deliver log messages from `LogObject`s to receivers.
You create one or more [receivers](#receiver-types) and one or more `LogObject`s. Messages emitted by any `LogObject` are automatically delivered to every attached receiver — receivers do not need to be wired to loggers by hand. The receiver is responsible for storing / displaying the messages; `LogObject`s themselves do not retain what they emit.

Receivers subscribe to the shared `LogManager` singleton on construction. Late-subscribed receivers (created after some `LogObject`s already exist) automatically get a snapshot of the existing loggers so they can pre-populate their state.

---

## Installation
#### Dependencies
* Qt5 or Qt6 [click here for more informations.](https://github.com/KROIA/QT_cmake_library_template/blob/main/documentation/HowToUse.md#dependencies)
* Qt modules
  * Core
  * Widgets

#### How to build
* If you want to use the logger library in a project that is based on the [library template](https://github.com/KROIA/QT_cmake_library_template), you can simply copy the `Logger.cmake` file to your projects `dependencies` folder and reconfigure that project.

* If you want to use the logger library as standalone build and include the logger lib manually to your project, download the repository and run the `build.bat` or open the CMakeLists.txt using Visual Studio and build and install the library.

---

## How to use

#### Basic example
``` C++
#include <QApplication>
#include "Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // (1) Create a receiver
    Log::UI::NativeConsoleView plotter;

    // (2) Create a logger object
    Log::LogObject logger("logger OBJ");

    // (3) Optional: give it a color
    logger.setColor(Log::Colors::Console::Foreground::cyan);

    // (4) Emit messages
    logger.log("This is a simple info message");
    logger.logInfo("Also an info message");
    logger.logWarning("A warning");
    logger.logError("An error");
    logger.logDebug("A debug line");
    logger.logCustom("A custom-level line");
    logger.logTrace("A trace line");

    logger.log("Level+color forwarded to the message", Log::Level::warning,
               Log::Colors::Console::Background::green);

    // Qt event loop drives the queued signal delivery to the receiver
    return app.exec();
}
```
1) Create a receiver that listens to the signals from the log objects.
2) The logger object is used to create log messages.
3) Each logger object can have an individual color which is used on the console.
4) Depending on the level, call the specific helper (`logInfo`, `logWarning`, ...) or the generic `log(text, level, color)`.

<div style="text-align: center;">
    <img src="documentation/Images/SimpleExampleOutput.png" alt="NativeConsoleView" width="500"/>
</div>

#### Nested loggers
Loggers form a tree. Pass a parent's `LoggerID` to nest a child under it — the tree view and file format preserve this structure.

``` C++
Log::LogObject root("app");
Log::LogObject net(root.getID(), "network");
Log::LogObject worker(net.getID(), "worker");

worker.logInfo("Connected");   // shown as app → network → worker → Connected
```

#### Custom colors
Every logger gets a default color; you can override it or set a color on a single message:
``` C++
Log::LogObject l("io");
l.setColor(Log::Colors::orange);            // this logger's default

l.logInfo("plain info");                    // orange in the views
l.log("This one is red",  Log::Level::info, Log::Colors::red); // per-message override
```

---

## Receiver types

The library ships several receivers. They all attach to the shared `LogManager` on construction, so you don't wire signals by hand — instantiate them and they receive every message.

* [`QConsoleView`](#qconsoleview) — flat table
* [`QTreeConsoleView`](#qtreeconsoleview) — parent/child tree
* [`QCombinedConsoleView`](#qcombinedconsoleview) — Table + Tree + Timeline + Stats tabs in one window
* [`QTimelineConsoleView`](#qtimelineconsoleview) — swimlane timeline with a density histogram
* [`QStatsConsoleView`](#qstatsconsoleview) — counters and rate dashboard
* [`NativeConsoleView`](#nativeconsoleview) — plain terminal output
* [`FilePlotter`](#fileplotter) — writes JSON log file to disk
* [Custom receiver](#custom-receiver-implementation) — subclass `AbstractReceiver` for anything else

#### QConsoleView
Flat table view. Columns: time, level, context, message. Rows can be filtered by level, context, date-time range, and message text.

``` C++
#include <QApplication>
#include "Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Log::UI::QConsoleView view;
    view.show();

    Log::LogObject logger("io");
    logger.logInfo("Hello");

    return app.exec();
}
```
<div style="text-align: center;">
    <img src="documentation/Images/QConsoleView.png" alt="QConsoleView" width="800"/>
</div>

#### QTreeConsoleView
Same data, arranged as a tree that preserves parent/child logger relationships.

``` C++
Log::UI::QTreeConsoleView tree;
tree.show();

Log::LogObject app("app");
Log::LogObject net(app.getID(), "network");

app.logInfo("Started");
net.logInfo("Connected");
```
<div style="text-align: center;">
    <img src="documentation/Images/QTreeConsoleView.png" alt="QTreeConsoleView" width="800"/>
</div>

#### QCombinedConsoleView
Four tabs — Table, Tree, Timeline, Stats — sharing one filter panel. Level and context checkboxes, the search bar, and the details pane apply to all four tabs at once.

``` C++
Log::UI::QCombinedConsoleView combined;
combined.show();
combined.setCurrentTab(Log::UI::QCombinedConsoleView::Tab::timeline);
```

Available tabs:

| `Tab` value | Description |
|---|---|
| `Tab::table`    | Flat table view (same as `QConsoleView`) |
| `Tab::tree`     | Tree view (same as `QTreeConsoleView`)   |
| `Tab::timeline` | Swimlane timeline (same as `QTimelineConsoleView`) |
| `Tab::stats`    | Counter dashboard (same as `QStatsConsoleView`) |

Saving visible messages (`saveVisibleMessages(path)`) exports rows from whichever row-based tab is active (table or tree). Timeline and stats tabs don't hold raw messages.

#### QTimelineConsoleView
Swimlane timeline: one horizontal lane per logger context, one dot per message drawn at its timestamp. Dot color = the lane's color so different loggers stay visually distinct; warnings and errors get a colored ring in the level color and grow with severity.

``` C++
Log::UI::QTimelineConsoleView timeline;
timeline.show();
```

Interactions:

| Action | Result |
|---|---|
| Left-drag inside the density histogram at the bottom | Rubber-band zoom to that time range |
| Mouse wheel over the timeline                        | Zoom in / out, anchored at the cursor |
| Right-click anywhere                                 | Reset to follow-live |
| Left-click a dot                                     | Pin a popup callout with the message text, anchored to that timestamp |
| Left-click a pinned dot again                        | Remove the callout |
| Hover a dot                                          | Tooltip with context, timestamp, level, message |
| "Window (s)" spinbox                                 | Set the visible time span |
| "Follow live" checkbox                               | Auto-scroll to newest |

A **density histogram** below the swimlanes shows message counts per time bucket, stacked and colored by level. It honors the current level / context / text filters. Toggle it with `setFeatureEnabled(HistogramStrip, false)`; toggle drag-to-zoom with `setFeatureEnabled(HistogramZoom, false)`.

#### QStatsConsoleView
Counter dashboard: total messages, rolling msg/s rate, per-level bar chart, per-context bar chart. Level bars scale to the busiest level; context bars scale to the busiest context so relative volumes read at a glance. Disabled contexts appear struck-through; darkened bars indicate filtered-out levels.

``` C++
Log::UI::QStatsConsoleView stats;
stats.show();
```

Nothing is emitted by user actions on this view — it is a read-only overview.

#### NativeConsoleView
Basic terminal output; no tree, no filtering.
``` C++
Log::UI::NativeConsoleView console;
console.show();
```
<div style="text-align: center;">
    <img src="documentation/Images/NativeConsoleView.png" alt="NativeConsoleView" width="500"/>
</div>

#### FilePlotter
Writes messages as JSON to a file. See [file format](#file-format).
``` C++
Log::FilePlotter plotter("outputFile.log");
```

#### Custom receiver implementation
Subclass `AbstractReceiver` and override the callbacks you care about:
``` C++
#include "Logger.h"

class CustomReceiver : public Log::AbstractReceiver
{
public:
    CustomReceiver() { /* automatically subscribes on construction */ }

protected:
    void onNewLogger(Log::LogObject::Info info) override        { /* ... */ }
    void onLoggerInfoChanged(Log::LogObject::Info info) override { /* ... */ }
    void onLogMessage(Log::Message message) override            { /* ... */ }
    void onChangeParent(Log::LoggerID childID,
                        Log::LoggerID newParentID) override      { /* ... */ }
};
```
You don't need to connect signals manually — the base class handles the subscription and delivers every message on the receiver's thread via queued connections.

---

## View features

Every view that derives from `QAbstractLogWidget` (all Qt-widget views above) shares the same feature set.

#### Search bar
An always-visible text field filters messages by substring (case-insensitive).

* Prefix the text with `!` to **exclude** matches instead of including them: `!heartbeat` hides every message containing "heartbeat".
* Toggle the `.*` checkbox to interpret the text as a **regular expression** (also case-insensitive).

``` C++
view.setSearchTextProgrammatic("!heartbeat", /*regex*/ false);
```

#### Find next / previous
The `▲` / `▼` buttons next to the search box jump the row selection to the next / previous matching row. `Enter` in the search box triggers "next". A count label shows how many rows currently match; it reads `0` when no search text is set.

Programmatically:
``` C++
view.findNext(/*forward=*/true);
int n = view.matchCount();
```

#### Level and context filters
The left-hand sidebar has one checkbox per level (`trace`, `debug`, `info`, `warning`, `error`, `custom`) and one per known logger context. Programmatic access:

``` C++
view.setLevelEnabled(Log::Level::trace, false);   // hide trace messages
view.setContextEnabled(logger.getID(), false);    // hide one context
view.soloContext(logger.getID());                 // hide everything else
view.hideContext(logger.getID());                 // hide just this one
```

#### Date-time filter
Enable the date-time filter in the sidebar, set min / max via the date-time widgets, and pick a range type: `before`, `after`, `between`, `equal`. Only messages whose timestamp satisfies the predicate are shown.

#### Details pane
A read-only pane below the content shows the currently selected row's full details: context name and ID, level (colored), timestamp in both human-readable and epoch-ms form, and the full multi-line message body. It updates on selection change and preserves whitespace in the message body.

#### Row context menu
Right-click a row in the table, tree, or combined view:

| Menu item | Action |
|---|---|
| Copy message text                | Copy plain text of the message to the clipboard |
| Copy row as JSON *(table only)*  | Copy the message serialized as JSON |
| Solo this context                | Hide all other contexts |
| Hide this context                | Hide just this context |
| Hide messages like this          | Sets the search box to `!<full message text>` |

#### In-cell text selection and copy
Click a cell to enter text-selection mode — drag inside the cell to select text (individual characters, not just whole rows). A double-click anywhere on a row copies the full message text to the clipboard. Selection survives incoming messages.

#### Feature flags
Turn individual UI features on or off per view instance. Useful for building a minimal read-only viewer or, conversely, ensuring the full toolset shows up.

``` C++
using F = Log::UIWidgets::QAbstractLogWidget;

// Strip a view down to its content only:
view.setFeatureEnabled(F::SearchBar,          false);
view.setFeatureEnabled(F::SearchRegexCheckBox, false);
view.setFeatureEnabled(F::FindNextPrev,       false);
view.setFeatureEnabled(F::MatchCount,         false);
view.setFeatureEnabled(F::DetailsPane,        false);
view.setFeatureEnabled(F::RowContextMenu,     false);
// Timeline-only flags:
view.setFeatureEnabled(F::HistogramStrip,     false);
view.setFeatureEnabled(F::HistogramZoom,      false);

bool on = view.isFeatureEnabled(F::SearchBar);
```

Flag applicability:

| Feature | Table | Tree | Combined | Timeline | Stats |
|---|:---:|:---:|:---:|:---:|:---:|
| `SearchBar` / `SearchRegexCheckBox` / `MatchCount` / `FindNextPrev` | ✓ | ✓ | ✓ | ✓ (search only) | ✓ (search only) |
| `DetailsPane`      | ✓ | ✓ | ✓ | — | — |
| `RowContextMenu`   | ✓ | ✓ | ✓ | — | — |
| `HistogramStrip` / `HistogramZoom` | — | — | — | ✓ | — |

#### Sub-widget toggles
For coarser stripping, hide entire sidebar sections:
``` C++
using SW = Log::UIWidgets::QAbstractLogWidget;

view.disableSubWidget(SW::settingsFrame);
view.disableSubWidget(SW::logLevelFilter);
view.disableSubWidget(SW::contextFilter);
view.disableSubWidget(SW::dateTimeFilter);
view.disableSubWidget(SW::editFrame);
// re-enable:
view.enableSubWidget(SW::logLevelFilter);
```

---

## Persisting logs

#### Saving visible messages
Any row-based view can dump its currently-visible messages to a JSON file:
``` C++
view.saveVisibleMessages("session.log");
```
"Visible" means: whatever passes the current level, context, date-time and text filters. Loggers that don't appear in any visible message are omitted.

The **Save** button in the sidebar does the same via a file picker.

#### Loading a log file
Any row-based view can load a file previously saved by this library:
``` C++
view.loadMessagesFromFile("session.log");
```
Logger metadata (name, color, parent) is restored so the tree view rebuilds the original hierarchy.

#### File format
A logger file is a JSON array. First element: library metadata. Next: one entry per logger context (id, name, color, parent, creation time). Then: one entry per message.

Timestamps are stored as **milliseconds since Unix epoch** (numeric), which is compact and easy to parse externally. Older files with formatted date-time strings still load — the reader accepts both.

``` json
[
  {
    "levelInfo": { "Trace": 0, "Debug": 1, "Info": 2, "Warning": 3, "Error": 4, "Custom": 5 },
    "name": "Logger",
    "version": "01.01.0000"
  },
  {
    "id": 1,
    "parentId": 0,
    "name": "app",
    "color": "#FFAA33",
    "enabled": true,
    "creationTime": 1735689305973
  },
  {
    "id": 2,
    "parentId": 1,
    "name": "network",
    "color": "#00CCFF",
    "enabled": true,
    "creationTime": 1735689305974
  },
  {
    "id": 1,
    "level": 2,
    "color": "#FFAA33",
    "text": "Started",
    "dateTime": 1735689306100
  },
  {
    "id": 2,
    "level": 3,
    "color": "#FFEE00",
    "text": "Connection lost",
    "dateTime": 1735689307250
  }
]
```
