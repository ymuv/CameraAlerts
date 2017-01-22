#ifndef NO_USE_PROFILER
#include <opencv2/core/core.hpp>
#include <iostream>

#include "logging/CClockExecution.hpp"
#include "logging/Log.hpp"
//TODO: logs macros to file
#define FILENAME_UNIX(arg) (arg)? (strrchr(arg, '/') ? strrchr(arg, '/') + 1 : arg) : arg
#define FILENAME_WIN(arg) (arg)? (strrchr(arg, '\\') ? strrchr(arg, '\\') + 1 : arg): arg

CClockExecution::CClockExecution()
{
#ifdef USE_PROFILER
   mTimeStart =  (double) cv::getTickCount();
   mDiffTime = mTimeStart;
#endif
}

void CClockExecution::printTime(const char* func, const char* file, int line, const std::string& message)
{
#ifdef USE_PROFILER
    double time = (double) cv::getTickCount();
    double t = 1000*(time - mTimeStart)/cv::getTickFrequency();
    double lastDiffTime = 1000*(time - mDiffTime)/cv::getTickFrequency();
    std::string s = FILENAME_WIN(FILENAME_UNIX(file));
    std::cout << message << " " << "time " << t << " diff: " << lastDiffTime
              << " [" << s << ":" << line << "]"
              << " (" << func << ")"
              << std::endl;
    mDiffTime = (double) cv::getTickCount();
#endif
}

void CClockExecution::printTimeAndReset(const char* func, const char* file, int line, const std::string& message)
{
#ifdef USE_PROFILER
    double t = 1000*((double)cv::getTickCount() - mTimeStart)/cv::getTickFrequency();
    LOG_WARN << func << ":" << line << message <<  " " << "time " << t;
    mTimeStart =  (double) cv::getTickCount();
#endif
}

void CClockExecution::reset()
{
#ifdef USE_PROFILER
    mTimeStart =  (double) cv::getTickCount();
#endif
}
#endif


