#pragma once
//TODO: to common/

#ifndef NO_USE_PROFILER
#include <string>
#define INIT_PROFILER(profilerName) CClockExecution profilerName
#define PROFILER_PRINT_TIME(profilerName) profilerName.printTime(__FUNCTION__, __FILE__, __LINE__)
#define PROFILER_PRINT_TIME_AND_RESET(profilerName) profilerName.printTimeAndReset(__FUNCTION__, __FILE__, __LINE__)
#define PROFILER_RESET(profilerName) profilerName.printTimeAndReset(__FUNCTION__, __FILE__, __LINE__)
#define PROFILER_PRINT_TIME_AND_MESSAGE(profilerName, message) profilerName.printTime(__FUNCTION__, __FILE__, __LINE__, message)
#define PRINT_TIME_AND_RESET(profilerName) profilerName.printTimeAndReset(__FUNCTION__, __FILE__, __LINE__)

class CClockExecution
{
public:
    CClockExecution();
    void printTime(const char* func, const char* file, int line, const std::string& message="");
    void reset();
    void printTimeAndReset(const char* func, const char* file, int line, const std::string& message="");
private:
    double mTimeStart;
    double mDiffTime;
};

#else
#define INIT_PROFILER(...)
#define PROFILER_PRINT_TIME(...)
#define PROFILER_PRINT_TIME_AND_RESET(...)
#define PROFILER_PRINT_TIME_AND_MESSAGE(...)
#endif
