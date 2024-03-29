#pragma once
#include "Logger_base.h"
#include <string>

namespace Log
{
	class LOGGER_EXPORT Profiler
	{
		public:
		static void startProfiler();
		static void stopProfiler(const std::string &profileFilePath);
	};
}