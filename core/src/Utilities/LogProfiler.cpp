#include "Utilities/LogProfiler.h"

namespace Log
{
    void Profiler::startProfiler()
    {
#ifdef LOGGER_PROFILING
        EASY_PROFILER_ENABLE;
#endif
    }
    void Profiler::stopProfiler(const std::string &profileFilePath)
    {
#ifdef LOGGER_PROFILING
        profiler::dumpBlocksToFile(profileFilePath.c_str());
#else
        LOGGER_UNUSED(profileFilePath);
#endif
    }
}