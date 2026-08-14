// @file Logger.h
// @brief Main public header for the library.
//
// Include this single header to access the entire public API.
// Add your own public headers inside USER_SECTION 2 so that
// consumers only need `#include "Logger.h"`.
#pragma once

/// USER_SECTION_START 1

/// USER_SECTION_END

#include "Logger_info.h"

/// USER_SECTION_START 2
// Logger Types
#include "LogObject.h"
#include "LogManager.h"

// Receivers
#include "AbstractReceiver.h"
//	 Console views
#include "ui/NativeConsoleView.h"
#include "ui/QTreeConsoleView.h"
#include "ui/QConsoleView.h"
#include "ui/QCombinedConsoleView.h"
#include "ui/QStatsConsoleView.h"
#include "ui/QVerticalTimelineView.h"
#include "ui/StaticObjs.h"
//	 File
#include "FilePlotter.h"


// Filters
#include "Filter/LoggerIDFilter.h"
 

// Utilities
#include "Utilities/Resources.h"
#include "Utilities/QtCompat.h"

/// USER_SECTION_END