#include <algorithm>
#include <chrono>
#include <thread>
#include <opencv2/highgui/highgui.hpp>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCommandLineParser>

//for debug
#include <fstream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __BUILD_MOTION_ALGO__

#include "Image/motionAlgorithms/CFaceDetect.hpp"
#include "Image/motionAlgorithms/CMoveDetect.hpp"
#include "Image/motionAlgorithms/CMoveDetectBitwiseAnd.hpp"
#include "Image/motionAlgorithms/CBackgroundSubtractorTest.hpp"
#include "Image/motionAlgorithms/CPHash.hpp"
#include "Image/motionAlgorithms/CPHashConfig.hpp"

#endif //__BUILD_MOTION_ALGO__

#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "Image/videoInput/CInputConfig.hpp"
#include "Text/CTextBuffer.hpp"
#include "common/CAlertNotification.hpp"
#include "common/CAlgoConfig.hpp"
#include "common/CConfig.hpp"
#include "common/CDateTime.hpp"
#include "common/CMainFunctions.hpp"
#include "common/CMemoryUssage.hpp"
#include "common/CVersion.hpp"
#include "common/private/CXMLParser.hpp"
#include "conditions/CConditionExpression.hpp"
#include "logging/CClockExecution.hpp"
#include "logging/Log.hpp"
#include "serial/CSerialPortConfig.hpp"

#ifdef __linux__
#include <execinfo.h>
#include <unistd.h>
#elif _WIN32
#include <Windows.h>
#include <process.h>
#include <DbgHelp.h>
#endif

QCoreApplication* CMainFunctions::mpApplication = nullptr;
int CMainFunctions::sPid = 0;
std::string CMainFunctions::sRunTime;

namespace
{
bool sIsProcessFirstAlertOnly = true; //false - for debug

void parseCmdOptions(int argc,
                     char** argv,
                     const std::string& binPath,
                     QString& logLevelStr,
                     QStringList& configPath,
                     CArgumentConfig& argumentConfig)
{
    //TODO: parse LOG file
    QFile file(argv[0]);
    auto dir = QFileInfo(file.fileName()).absoluteDir().path();

    if (!CMainFunctions::mpApplication)
    {
        CMainFunctions::mpApplication =
                new QCoreApplication(argc, argv);
    }
    QCoreApplication::setApplicationName("cameraAlerts");

    QCommandLineParser parser;
    parser.setApplicationDescription(
                QString("Camera allerts application\n")
                + CVersion::getProjectBuildVersion() + "\n" + CVersion::getProjectBuildTime());
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption logLevel(QStringList() << "l" << "logLevel",
                                "log level to output(trace,debug,info,warning,error,fatal)",
                                "logLevel", "info");

    QCommandLineOption debugMode(QStringList() << "d" << "debugMode",
                                 "set script and videoSaveDir to empty string");

    QCommandLineOption noSMTP(QStringList() << "s" << "noSend",
                              "not send emails and telegram");

    QCommandLineOption configs(QStringList({"c, config"}),
                               "configFiles");
    QCommandLineOption noAlgThreads(QStringList() << "n" << "noAlgThread", "not run algorithms in thread");

    QString path1(dir + "/config.xml");
    QString path2 = QString::fromStdString(binPath + "/config.xml");
    QString path3 = QString(CMAKE_ROOT_PATH) + "/config.xml";

    //if put QString to initializer list, QCore crash on windows.
    configs.setDefaultValues(
                QStringList({path1,
                             path2,
                             path3}));

    QCommandLineOption visualize("V","If true - ignore file settings debug image show");
    QCommandLineOption visualizeAlgo("algoDebug","If true - ignore file settings debug image algo show");

    parser.addOption(logLevel);
    parser.addOption(configs);
    parser.addOption(visualize);
    parser.addOption(visualizeAlgo);
    parser.addOption(debugMode);
    parser.addOption(noSMTP);
    parser.addOption(noAlgThreads);

    parser.process(*CMainFunctions::mpApplication);
    argumentConfig.mIsVisulize = parser.isSet(visualize);
    argumentConfig.mIsVisulizeAlgoImages = parser.isSet(visualizeAlgo);
    argumentConfig.mIsDebug = parser.isSet(debugMode);
    argumentConfig.mNotNotUseSMTP = parser.isSet(noSMTP);
    argumentConfig.mNotUseThreads = parser.isSet(noAlgThreads);

    logLevelStr = parser.value(logLevel);
    configPath =parser.values(configs);
}
}

void CMainFunctions::parseParams(int argc, char** argv, const std::string& installPath)
{
    init();
    CConfig& config = CConfig::getInstance();
    CArgumentConfig argConfig;
    QStringList configPaths;
    QString logLevel;
    parseCmdOptions(argc, argv, installPath, logLevel, configPaths, argConfig);

    NLogger::CLogger::initLogging(logLevel);

    LOG_INFO << CVersion::getProjectBuildVersion();
    LOG_INFO << CVersion::getProjectBuildTime();
    LOG_INFO << CVersion::getProjectLastCommitDateTime();

    for (int i = 0; i < configPaths.size(); i++)
    {
        if (QFile(QString(configPaths[i])).exists())
        {
            LOG_INFO << "parse " << configPaths[i];
            CXMLParser::parseXMLFromFile(
                        configPaths[i].toStdString(),
                        config,
                        CAlgoConfig::getInstance(),
                        argConfig);
            break;
        }
    }

    NLogger::CLogger::initLogOutFile(argConfig.mFileName);

    if (!config.mRunExternalCommandOnStart.empty())
    {
        auto res = system(config.mRunExternalCommandOnStart.c_str());
        if (res)
        {
            LOG_ERROR << "result script execute: " << res;
            LOG_ERROR << config.mRunExternalCommandOnStart;
        }
    }
}

void CMainFunctions::addAlgoToVector(const pAlertAlgorithm algo,
                                     algorighmListPtr& algorithms)
{
    algorithms.push_back(algo);
    LOG_DEBUG << "add algorithm " << algo->getAlgorithmName() << "for id" << algo->getBufferId();
}

bool CMainFunctions::doTextConditions(
        CAlertNotification& alert,
        const NText::CTextPattern& pattern,
        NText::CTextPattern::tCeserialData parsedValue,
        bool isOwnBuffer)
{
    bool retValue = false;
    if (CConfig::getInstance().mRunStatus.mHomeStatus == CRunStatus::HomeStatus::HOME_OUT)
    {
        if (pattern.mIsProcess)
        {
            auto condType = NAlarms::ALERT_TYPE::NO_CONDITION;
            std::string conditionType;
            if ((!pattern.mIsReverse && ( parsedValue >= pattern.mTreshholdAlert)) ||
                    (pattern.mIsReverse && (parsedValue <= pattern.mTreshholdAlert)))
            {
                condType = NAlarms::ALERT_TYPE::CONDITION_ALERT;
                conditionType = "Alert";
            }
            else if ((!pattern.mIsReverse && (parsedValue >= pattern.mTreshholdWarning)) ||
                    (pattern.mIsReverse && (parsedValue <= pattern.mTreshholdWarning)))
            {
                condType = NAlarms::ALERT_TYPE::CONDITION_WARNING;
                conditionType = "Warning";
            }

            if (condType != NAlarms::ALERT_TYPE::NO_CONDITION)
            {
                retValue = true;
                if (isOwnBuffer)
                    NText::CTextBuffer::getInstance().append(pattern, parsedValue, true);

                LOG_TEXT_ALERT << conditionType + " in pattern " << pattern.mPattern << parsedValue
                               << "mem: " << CMemoryUssage::processMemoryUssage()
                               << "max mem: " << CMemoryUssage::processMaxMemoryUssage();
                alert.sendAlert(
                            CImageBuffer::getBufferInstance(),
                            pattern.mBuffersForWrite,
                            "PATTERN_" + pattern.mPattern.toStdString(),
                            conditionType + " in pattern "+ pattern.mPattern.toStdString()
                            + " _" + std::to_string(parsedValue),
                            condType);
            }
            else
            {
                if (isOwnBuffer)
                    NText::CTextBuffer::getInstance().append(pattern, parsedValue, false);

                alert.checkAndSendReplyAndForce(CImageBuffer::getBufferInstance(), pattern.mBuffersForWrite);
            }
        }
        else
        {
            if (isOwnBuffer)
                NText::CTextBuffer::getInstance().append(pattern, parsedValue, false);
        }
    }
    return retValue;
}

void CMainFunctions::doTextAlgo(
        const QByteArray& data,
        CAlertNotification& alert)
{
    QString receivedStr(data);
    for (const auto& str : receivedStr.split(QRegExp("<html>|</html>|<br/?>|[\r\n][\r\n]*"), QString::SkipEmptyParts))
    {
        QString strTrimmed = str.trimmed().replace(": ", " ");

        for (size_t i = 0; i < CConfig::getInstance().mSerialPatterns->mPatterns.size(); i++)
        {
            const auto& pattern =  CConfig::getInstance().mSerialPatterns->mPatterns[i];
            if (strTrimmed.startsWith(pattern.mPattern))
            {
                QString status = strTrimmed;
                status = status.replace(pattern.mPattern, "").trimmed();
                bool ok;
                NText::CTextPattern::tCeserialData parsedValue = status.toFloat(&ok);
                if (ok)
                {
                    NText::CTextBuffer::getInstance().append(pattern, parsedValue);
                    doTextConditions(alert, pattern, parsedValue, true);
                    if (CConfig::getInstance().mRunStatus.mIsRunConditions)
                    {
                        NCondition::CConditionExpression::getInstance()->checkAllExpression();
                    }
                }

            }
        }
    }
}

void CMainFunctions::doTextAlgo(CAlertNotification& alert)
{
    NText::CTextBuffer& textBuffer = NText::CTextBuffer::getInstance();
    for (size_t i = 0; i < CConfig::getInstance().mSerialPatterns->mPatterns.size(); i++)
    {
        const auto& pattern =  CConfig::getInstance().mSerialPatterns->mPatterns[i];
        if (textBuffer.getMap().contains(pattern.mPattern))
        {
            if (!textBuffer.getMap()[pattern.mPattern].empty())
            {
                auto& list = textBuffer.getMap()[pattern.mPattern];
                auto data = list.last().mData;
                if (doTextConditions(alert, pattern, data, false))
                {
                    list.last().mIsAlert = true;
                }

            }
        }
    }
    if (CConfig::getInstance().mRunStatus.mIsRunConditions)
    {
        NCondition::CConditionExpression::getInstance()->checkAllExpression();
    }
}

void CMainFunctions::makeDirIfNotExist(const std::string& dir)
{
    if (!dir.empty())
    {
        if (!QDir(dir.c_str()).exists())
        {
            if (!QDir().mkpath(dir.c_str()))
            {
                LOG_ERROR << " failed to create dir " << dir;
            }
        }
    }
}

void CMainFunctions::addAlgorithm(algorighmListPtr& algorithms)
{
#ifdef __BUILD_MOTION_ALGO__
    using namespace NAlgorithms;
    const CAlgoConfig& algoConfig = CAlgoConfig::getInstance();
    for (size_t i = 0; i < CImageBuffer::getBufferInstance().getBuffersSize(); i++)
    {
        const auto& algoCoef = CConfig::getInstance().mInputConfig->mInputList[i].mAlgoCoeficient;
        if (algoConfig.mPHashConfig->mIsRun)
        {
            auto algo = std::make_shared<CPHash>(*algoConfig.mPHashConfig, i);
            addAlgoToVector(algo, algorithms);
        }

        if (algoConfig.mMoveDetectConfig->mIsRun)
        {
            auto algo = std::make_shared<CMoveDetect>(
                        *algoConfig.mMoveDetectConfig, i, false, algoCoef);
            addAlgoToVector(algo, algorithms);
        }

        if (algoConfig.mMoveDetectBitwiseAnd->mIsRun)
        {
            algorithms.push_back(std::make_shared<CMoveDetectBitwiseAnd>(
                                     *algoConfig.mMoveDetectBitwiseAnd, i,
                                     false, algoCoef));
        }

        if (algoConfig.mMoveDetectBackgroundSubtractorTestConfig->mIsRun)
        {
            auto algo = std::make_shared<CMoveDetectBackgroundSubtractorTest>(
                        *algoConfig.mMoveDetectBackgroundSubtractorTestConfig, i, false, algoCoef);
            addAlgoToVector(algo, algorithms);
        }

        if (algoConfig.mMoveDetectBackgroundSubtractorTestColorConfig->mIsRun)
        {
            auto algo = std::make_shared<CMoveDetectBackgroundSubtractorTest>(
                        *algoConfig.mMoveDetectBackgroundSubtractorTestColorConfig, i, true, algoCoef);
            addAlgoToVector(algo, algorithms);
        }

        if (algoConfig.mFaceDetectConfig->mIsRun)
        {
            auto algo = std::make_shared<CFaceDetect>(
                        *algoConfig.mFaceDetectConfig, i);
            addAlgoToVector(algo, algorithms);
        }
        if (algoConfig.mMoveDetectConfigColor->mIsRun)
        {
            auto algo = std::make_shared<CMoveDetect>(
                        *algoConfig.mMoveDetectConfigColor, i, true, algoCoef);
            addAlgoToVector(algo, algorithms);
        }
    }
#endif
}

double CMainFunctions::getSleepTime(size_t bufferId)
{
    return CConfig::getInstance().mInputConfig->mInputList[bufferId].mSleepTime;
}

void CMainFunctions::sleep(double time)
{
    const static int msInSecond = 1000;
    sleepMiliseconds(int (time * msInSecond));
}

void CMainFunctions::sleepMiliseconds(int time)
{
    if (time > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
}

void CMainFunctions::sleepTo(double timeBegin, size_t bufferId)
{
    double runTime = ((double) cv::getTickCount() - timeBegin) / cv::getTickFrequency();

    double sleepTime = getSleepTime(bufferId);

    sleep(sleepTime - runTime);
}

void CMainFunctions::restartAlgo(size_t bufferId, algorighmListPtr& algorithms)
{
    for (size_t i = 0; i < algorithms.size(); i++)
    {
        if (algorithms[i]->getBufferId() == bufferId)
        {
            algorithms[i]->reset();
        }
    }
}

void CMainFunctions::doAlgo(
        CAlertNotification& alert,
        size_t bufferId,
        algorighmListPtr& algorithms,
        const std::string& imShowPrefix)
{
    if (!CConfig::getInstance().mRunStatus.mIsRunMotionAlgo)
    {
        return;
    }
    if (algorithms.size() == 0)
    {
        return;
    }

    INIT_PROFILER(algoCommonProfiler);
    static const std::string imageMaskName = "mask";
    CImageBuffer buff = CImageBuffer::getBufferInstance();
    const auto& buff2 = buff.getList(bufferId);

    CConfig& config = CConfig::getInstance();

    auto size =  buff2.size();
    if (size > NAlgorithms::IAlertAlgorithm::sMinAlgoFrameCount && buff.isFull(bufferId))
    {
        for (int i = 0; i < NAlgorithms::IAlertAlgorithm::sMinAlgoFrameCount; i++)
        {
            //                matLockers.at(i).reset(new CMatWithTimeStampLocker(*buff2[buff2.size() - i -1]));
        }
        auto lastMat = buff2[size - 1];
        cv::Mat mask;
        lastMat->getMat().copyTo(mask);

        std::string algoInfo, algoName;

        CImageBuffer::getBufferInstance().setHasAlert(bufferId, false);

        for (size_t i = 0; i < algorithms.size(); i++)
        {
            if (algorithms[i]->getBufferId() == bufferId)
            {
                INIT_PROFILER(algoProfiler);
                bool result = algorithms[i]->hasAlertWithSkip(buff2, mask, algoInfo);
                if (result)
                {
                    algoName = algorithms[i]->getAlgorithmName();
                    LOG_ALGO_ALERT << " alert in id"
                                   << lastMat->getId()
                                   << "(buffer" << bufferId  << ","
                                   << buff.getBufferInfo(bufferId) << ")"
                                   << algorithms[i]->getAlgorithmName()
                                   << "mem: " << CMemoryUssage::processMemoryUssage()
                                   << "max mem: " << CMemoryUssage::processMaxMemoryUssage();

                    lastMat->setMask(mask);

                    alert.sendAlert(
                                buff,
                                std::vector<size_t>{bufferId},
                                algorithms[i]->getAlgorithmName(),
                                algoInfo,
                                NAlarms::ALERT_TYPE::CONDITION_ALERT);
                    CImageBuffer::getBufferInstance().setHasAlert(bufferId, true);

                    if (sIsProcessFirstAlertOnly)
                    {
                        break;
                    }
                }
                PROFILER_PRINT_TIME_AND_MESSAGE(algoProfiler, algorithms[i]->getAlgorithmName()
                                                +" " +  std::to_string(bufferId));
            }
        }

        alert.checkAndSendReplyAndForce(buff, std::vector<size_t> {bufferId});

        if (config.mDebugShow.mIsShowDebugImg)
        {
            if(config.mDebugShow.mIsWriteInfoToMask)
            {
                cv::Mat copyMat;
                mask.copyTo(copyMat);
                const auto lastMat = buff.getLast(bufferId);
                auto prevId =lastMat->getId();
                if (buff.size(bufferId) > 1)
                {
                    prevId = buff.getLast2(bufferId)->getId();
                }
                auto diffId = lastMat->getId() - prevId;
                CMatWithTimeStamp::writeText(
                            copyMat,
                            lastMat->getDateTimeWithMS() + " diff " + std::to_string(diffId)
                            + " " + std::to_string(copyMat.cols)
                            + " " + std::to_string(copyMat.rows),
                            std::to_string(lastMat->getId()));
                if (!algoInfo.empty())
                {
                    CMatWithTimeStamp::writeText(
                                copyMat,
                                algoName  + "\n" + algoInfo);
                }
                cv::imshow(imShowPrefix + imageMaskName + std::to_string(bufferId), copyMat);
            }
            else
            {
                cv::imshow(imShowPrefix + imageMaskName + std::to_string(bufferId), mask);
            }
            cv::waitKey(1);
        }
        if (config.mDebugShow.mIsShowAlgoImages)
        {
            for (size_t i = 0; i < algorithms.size(); i++)
            {
                if ((algorithms[i]->getBufferId() == bufferId)
                        && !algorithms[i]->getMask().empty())
                {
                    std::string imgName =
                            std::to_string(i) + "_"
                            + algorithms[i]->getAlgorithmName();

                    cv::imshow(imgName, algorithms[i]->getMask());
                }
                else
                {
                    std::string imgName = algorithms[i]->getAlgorithmName();
                    LOG_DEBUG << imgName << "is emtpy";
                }
            }
            cv::waitKey(1);
        }
    }
    else
    {
        if (config.mDebugShow.mIsShowDebugImg &&
                !buff.empty(bufferId))
        {
            const cv::Mat tmpMat = buff.getLast(bufferId)->getMat();
            auto lastId = buff2[size - 1]->getId();
            auto prevId = lastId;
            if (buff.size(bufferId) > 2)
            {
                prevId = buff2[size -2]->getId();
            }
            //don't edit original image
            cv::Mat mat;
            tmpMat.copyTo(mat);
            std::string str;
            str += std::to_string(buff.size(bufferId)) + " of " + std::to_string(buff.maxSize(bufferId)) + "  "
                    + buff.getLast(bufferId)->getDateTimeWithMS()
                    + " ; diff: " + std::to_string(lastId - prevId);

            CMatWithTimeStamp::writeText(
                        mat,
                        "wait..." + str,
                        std::to_string(buff.getLast(bufferId)->getId()));
            cv::imshow(imShowPrefix + imageMaskName + std::to_string(bufferId), mat);
            cv::waitKey(1);
        }
    }
#ifdef __BUILD_EXPRESSION__
    if (CConfig::getInstance().mRunStatus.mIsRunConditions)
    {
        NCondition::CConditionExpression::getInstance()->doExpression();
    }
#endif

}

void CMainFunctions::printTrace(int)
{
#ifdef __linux__
#define BACKTRACE_ARR_SIZE 30
    static bool isCalled = false;
    if (!isCalled)
        isCalled = true;
    else
    {
        throw "";
    }
    void* array[BACKTRACE_ARR_SIZE];
    size_t size;
    char** strings;
    size_t i;

    std::ofstream outfile;
    outfile.open("crash_log.txt", std::ios_base::app);
    outfile << "\n\n crash app" << std::endl;
    outfile << CDateTime::getDateTimeAsString() << std::endl;

    size = backtrace (array, BACKTRACE_ARR_SIZE);
    strings = backtrace_symbols (array, size);
    std::cout << "\nBackStack:" << std::endl;

    for (i = 0; i < size; i++)
    {
        std::cout << strings[i] << std::endl;
        outfile << strings[i] << std::endl;
        outfile.flush();
    }

    outfile.close();

    free (strings);
    exit(1);
#elif _WIN32 //not tested
/*
    unsigned int   i;
    void         * stack[ 100 ];
    unsigned short frames;
    SYMBOL_INFO  * symbol;
    HANDLE         process;

    process = GetCurrentProcess();

    SymInitialize( process, NULL, TRUE );

    frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
    symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    for( i = 0; i < frames; i++ )
    {
        SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );

        printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
    }

    free( symbol );
    */
#endif
}

void CMainFunctions::init()
{
    sRunTime = CDateTime::getDateTimeAsString();
#ifdef __linux__
    sPid = getpid();
#elif _WIN32
    sPid = _getpid();
#endif
}

void CMainFunctions::doAlgoAsync(
        CAlertNotification& alert,
        size_t bufferId,
        algorighmListPtr& algorithms,
        std::mutex& mutex,
        const std::string& imShowPrefix)
{
    std::unique_lock<std::mutex> lockForce(mutex, std::try_to_lock);

    if(lockForce.owns_lock())
    {
        doAlgo(alert, bufferId, algorithms, imShowPrefix);
    }
}

bool CMainFunctions::isMatTypeSame(const cv::Mat& mat1, const cv::Mat& mat2)
{
    if (mat1.cols == mat2.cols &&
             mat1.rows == mat2.rows &&
             mat1.dims == mat2.dims)
    {
        return true;
    }
    return false;
}
