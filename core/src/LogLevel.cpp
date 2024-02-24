#include "LogLevel.h"
#include "Utilities/Resources.h"

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
		case Level::trace:		{ return Resources::getIconTrace(); }
		case Level::debug:		{ return Resources::getIconDebug(); }
		case Level::info:		{ return Resources::getIconInfo();	}
		case Level::warning:	{ return Resources::getIconWarning(); }
		case Level::error:		{ return Resources::getIconError(); }
		}

		const QIcon dummy;
		return dummy;
	}
#endif
}