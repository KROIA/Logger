#pragma once
#include "Logger_base.h"
#include "LoggerTypes/AbstractLogger.h"


namespace Log
{
	class LOGGER_EXPORT Export
	{
	public:
		static bool saveToFile(const std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list, const std::string& file);
	
	private:

	};
}