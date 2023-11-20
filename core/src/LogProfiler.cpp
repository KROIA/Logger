#include "LogProfiler.h"

namespace Log
{
    void Profiler::startProfiler()
    {
#ifdef JD_PROFILING
        EASY_PROFILER_ENABLE;
#endif
    }
    void Profiler::stopProfiler(const std::string profileFilePath)
    {
#ifdef JD_PROFILING
        profiler::dumpBlocksToFile(profileFilePath.c_str());
#endif
    }
}