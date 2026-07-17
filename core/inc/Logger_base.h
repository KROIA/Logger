// @file Logger_base.h
// @brief Internal base header included by every library source file.
//
// Pulls in the global export/import macros, debug/profiling utilities,
// and library metadata. Include this in your own library headers instead
// of including the individual headers separately.
#pragma once

/// USER_SECTION_START 1

/// USER_SECTION_END

#include "Logger_global.h"
#include "Logger_debug.h"
#include "Logger_info.h"

/// USER_SECTION_START 2
namespace Log
{
	namespace UI
	{
		enum ConsoleViewType
		{
			nativeConsoleView,
			qConsoleView,
			qTreeConsoleView
		};
	}
	typedef unsigned int LoggerID;
}
/// USER_SECTION_END