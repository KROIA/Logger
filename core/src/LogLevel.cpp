#include "LogLevel.h"

namespace Log
{
	const std::string& getLevelStr(Level l)
	{
		switch (l)
		{
		case Level::trace:   { const static std::string str = " Trace:   "; return str; }
		case Level::debug:   { const static std::string str = " Debug:   "; return str; }
		case Level::info:    { const static std::string str = " Info:    "; return str; }
		case Level::warning: { const static std::string str = " Warning: "; return str; }
		case Level::error:   { const static std::string str = " Error:   "; return str; }
		}
		static std::string str;
		str = "Unknown debug level: " + std::to_string(l);
		return str;
	}
#ifdef LOGGER_QT
	const QIcon& getIcon(Level logLevel)
	{
		switch (logLevel)
		{
		case Level::trace: { static const QIcon icon(":/icons/trace.png"); return icon; }
		case Level::debug: { static const QIcon icon(":/icons/debug.png"); return icon; }
		case Level::info: { static const QIcon icon(":/icons/info.png"); return icon; }
		case Level::warning: { static const QIcon icon(":/icons/warning.png"); return icon; }
		case Level::error: { static const QIcon icon(":/icons/error.png"); return icon; }
		}

		const QIcon dummy;
		return dummy;
	}
#endif
}