#pragma once
#include <string>

namespace NLogger
{

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    LEVEL_ERROR,
    CRITICAL,
    FATAL,
    NOT_SET,
};

struct CLoggerConfig
{
    LogLevel mLogLevel;
    std::string mLogFileName;
};
}
