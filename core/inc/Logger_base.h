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
}
/// USER_SECTION_END