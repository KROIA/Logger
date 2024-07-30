#pragma once
#include "Logger_base.h"
#include "LogMessage.h"


namespace Log
{
	class LOGGER_EXPORT Export
	{
	public:
		static bool saveToFile(const std::unordered_map<LoggerID, std::vector<Message>>& contexts, const std::string& file);
	
	private:

	};
}