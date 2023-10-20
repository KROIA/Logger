#pragma once

#include "Logger_base.h"
#include "LogColor.h"

namespace Log
{
	enum Level
	{
		trace,
		debug,
		info,
		warning,
		error,
		custom
	};

	struct LOGGER_EXPORT LevelColors
	{
		Color trace;
		Color debug;
		Color info;
		Color warning;
		Color error;
		Color custom;
	};


}