#pragma once

#include "Logger_base.h"
#include "LogColor.h"
#include <string>

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

	inline std::string getLevelStr(Level l)
	{
		switch (l)
		{
		case Level::trace:   return " Trace:   ";
		case Level::debug:   return " Debug:   ";
		case Level::info:    return " Info:    ";
		case Level::warning: return " Warning: ";
		case Level::error:   return " Error:   ";
		}
		return "Unknown debug level: " + std::to_string(l);
	}


}