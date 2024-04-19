#pragma once

/// USER_SECTION_START 1

/// USER_SECTION_END

#include "Logger_info.h"

/// USER_SECTION_START 2
// Logger Types
#include "LoggerTypes/AbstractLogger.h"
#include "LoggerTypes/ContextLogger.h"

// Log Receivers
#include "ReceiverTypes/AbstractReceiver.h"
#include "ReceiverTypes/ContextReceiver.h"
#include "ReceiverTypes/ConsoleContextPlotter.h"

// ui
#include "ui/QContextLoggerTreeView.h"
#include "ui/QConsoleView.h"

// Utilities
#include "Utilities/Resources.h"

/// USER_SECTION_END