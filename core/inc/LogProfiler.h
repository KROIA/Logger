#pragma once
#include "Logger_base.h"

namespace Log
{
	class LOGGER_EXPORT Profiler
	{
		public:
		static void startProfiler();
		static void stopProfiler(const std::string profileFilePath);
	};
}