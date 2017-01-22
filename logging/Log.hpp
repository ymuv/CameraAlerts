#pragma once
#include <QDebug>

#include "CLoggerConfig.hpp"

QDebug operator<<(QDebug out, const std::string& str);

#define LOG_TRACE qDebug()
#define LOG_DEBUG qDebug()

#if QT_VERSION >= 0x50500
#define LOG_INFO  qInfo()
#else
#define LOG_INFO  qDebug()
#endif

#define LOG_WARN  qWarning()
#define LOG_ERROR qCritical()
#define LOG_FATAL LOG_ERROR

#define LOG_ALGO_ALERT LOG_INFO
#define LOG_TEXT_ALERT LOG_INFO

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define LOG_DEBUG_EVERY_COUNT(cnt, varName, message) do {\
    static size_t TOKENPASTE2(log_counter_, varName) = 0; \
    if ((TOKENPASTE2(log_counter_, varName)++ % cnt ) == 0) \
    {\
        LOG_DEBUG << message;\
    }} while(0)

#define LOG_VAR(var) #var << var

namespace NLogger
{
class CLogger
{
public:
    static CLogger& getInstance();
    static void initLogging(const QString& logLevel);
    static void initLogOutFile(const std::string& logFile);

    LogLevel getLogLevel() const;

private:
    CLogger();
    CLogger(CLogger const&) = delete;
    CLogger(CLogger&&) = delete;
    CLogger& operator=(CLogger const&) = delete;
    CLogger& operator=(CLogger&&) = delete;
private:
    LogLevel mLogLevel;
};
}
