#include <QDateTime>
#include <QFile>
#include <QFile>

#include "logging/Log.hpp"

namespace NLogger
{

#define FILENAME_UNIX(arg) (arg)? (strrchr(arg, '/') ? strrchr(arg, '/') + 1 : arg) : arg
#define FILENAME_WIN(arg) (arg)? (strrchr(arg, '\\') ? strrchr(arg, '\\') + 1 : arg): arg

static QFile* sLogFile = nullptr;

inline
void printMessage(
        const char* messagePrefix,
        const QByteArray& localMsg,
        const QMessageLogContext &context)
{
    std::string str;
    {
        auto dt = QDateTime::currentDateTime();
        QString timeStamp = dt.toString("yyyy-MM-dd hh:mm:ss.zzz");
        str = timeStamp.toStdString();
    }

    auto device = stderr;
    fprintf(device, "[%s] %s: %s (%s:%u)\n",
            str.c_str(),
            messagePrefix,
            localMsg.constData(),
            FILENAME_WIN(FILENAME_UNIX(context.file)),
            context.line);
    if (sLogFile && sLogFile->isOpen())
    {
        QTextStream logStream(sLogFile);
        logStream.flush();
        sLogFile->flush();
    }
    fflush(device);
}

void myMessageOutput(const QtMsgType type, const QMessageLogContext &context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    auto level = CLogger::getInstance().getLogLevel();

    switch (type)
    {
    case QtDebugMsg:
        if ((int) level <= (int) LogLevel::DEBUG)
            printMessage("D", localMsg, context);
        break;

#if QT_VERSION >= 0x50500
    case QtInfoMsg:
        if ((int) level <= (int) LogLevel::INFO)
            printMessage("I", localMsg, context);
        break;
#endif
    case QtWarningMsg:
        if ((int) level <= (int) LogLevel::WARNING)
            printMessage("W", localMsg, context);
        break;
    case QtCriticalMsg:
        if ((int) level <= (int) LogLevel::CRITICAL)
            printMessage("C", localMsg, context);
        break;
    case QtFatalMsg:
        if ((int) level <= (int) LogLevel::FATAL)
            printMessage("F", localMsg, context);
//        abort();
    }
}

CLogger& CLogger::getInstance()
{
    static CLogger instance;
    return instance;
}

void CLogger::initLogging(const QString& logLevel)
{
#define KEY_TO_MAP(key, shortKey, value) \
    { key, value}, { shortKey, value}

    LogLevel level;

    std::map<QString, LogLevel> xmap
            ({
                 KEY_TO_MAP("debug", "d", LogLevel::DEBUG),
                 KEY_TO_MAP("info", "i", LogLevel::INFO),
                 KEY_TO_MAP("warning", "w", LogLevel::WARNING),
                 KEY_TO_MAP("error", "e", LogLevel::LEVEL_ERROR),
                 KEY_TO_MAP("critical", "c", LogLevel::CRITICAL),
                 KEY_TO_MAP("fatal", "f", LogLevel::FATAL)
             });

    std::string infoStr;

    if (xmap.find(logLevel.toLower()) == xmap.end() )
    {
        infoStr = "log level \'" + logLevel.toStdString()
                + "\' not found, using warn level";
        qDebug() << infoStr;
        level = LogLevel::WARNING;
    }
    else
    {
        level = xmap[logLevel.toLower()];
        infoStr = "log level " +  logLevel.toStdString() +
                + " " + std::to_string((int) level);
    }

    qInstallMessageHandler(myMessageOutput);
    CLogger::getInstance().mLogLevel = level;

    LOG_INFO << infoStr;
}

void CLogger::initLogOutFile(const std::string& logFile)
{
    if (!logFile.empty())
    {
//        //log rotation
//        for (int i = 9; i >=0; i--)
//        {
////            QFile file(QString::fromStdString(logFile) + "." + QString::number(i));
//        }

        QFile oldLogFile(QString::fromStdString(logFile));
        oldLogFile.rename(QString::fromStdString(logFile) + "." + QString::number(0));

        if (!sLogFile)
        {
            sLogFile = new QFile(QString::fromStdString(logFile));
            if(!sLogFile->open(QIODevice::WriteOnly))
            {
                LOG_WARN << "fail open log file for writing";
            }
        }
    }
}

CLogger::CLogger()
{
}

LogLevel CLogger::getLogLevel() const
{
    return mLogLevel;
}
}

QDebug operator<<(QDebug out, const std::string& str)
{
    out << QString::fromStdString(str);
    return out;
}
