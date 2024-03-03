#pragma once

#include "Logger_base.h"
#include "LogColor.h"
#include <string>

#ifdef LOGGER_QT
#include <QIcon>
#endif

namespace Log
{
	enum Level
	{
		trace,
		debug,
		info,
		warning,
		error,
		custom,

		__count
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

	namespace Utilities
	{
	
		const std::string& getLevelStr(Level l);

#ifdef LOGGER_QT
		const QIcon& getIcon(Level logLevel);
#endif
	}


}