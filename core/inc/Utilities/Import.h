#pragma once
#include "Logger_base.h"
#include "LogObject.h"
#include <vector>
#include <memory>

namespace Log
{
	class LOGGER_API Import
	{
	public:
		static bool loadFromFile(std::vector<std::pair<LogObject::Info, std::vector<Message>>>& contexts, const std::string& file);




	private:
		static QJsonArray  parseLargeJson(const std::string& filePath);
	};
}