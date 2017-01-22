#pragma once

#include <memory>
#include <mutex>

#include "Image/motionAlgorithms/IAlertAlgorithm.hpp"
#include "Text/CTextPatterns.hpp"

namespace NAlgorithms
{
    class IAlertAlgorithm;
}
namespace NSerial
{
    struct CSerialPortConfig;
}

class QByteArray;
class QCoreApplication;

class CAlertNotification;
class CImageBuffer;
class CMatWithTimeStamp;

class CMainFunctions
{
public:
    CMainFunctions() = delete;
    static void printTrace(int);

    static void init();
    static void parseParams(int argc, char** argv, const std::string& installPath);

    //must be called after create buffers
    static void addAlgorithm(algorighmListPtr& algorithms);

    static double getSleepTime(size_t bufferId);
    static void sleep(double time);
    static void sleepMiliseconds(int time);
    static void sleepTo(double timeBegin, size_t bufferId =0);

    static void restartAlgo(size_t bufferId, algorighmListPtr& algorithms);

    static void doAlgo(
            CAlertNotification& alert,
            size_t bufferId,
            algorighmListPtr& algorithms,
            const std::string& imShowPrefix = "");

    static void doAlgoAsync(
            CAlertNotification& alert,
            size_t bufferId,
            algorighmListPtr& algorithms,
            std::mutex& mutex,
            const std::string& imShowPrefix = "");

    static bool isMatTypeSame(const cv::Mat& mat1, const cv::Mat& mat2);

    static QCoreApplication* mpApplication;

    static void addAlgoToVector(
          const pAlertAlgorithm algo, algorighmListPtr& algorithms);

    static void doTextAlgo(
            const QByteArray& data,
            CAlertNotification& alert);

    static void doTextAlgo(
            CAlertNotification& alert);

    static bool doTextConditions(
            CAlertNotification& alert,
            const NText::CTextPattern& pattern,
            NText::CTextPattern::tCeserialData parsedValue,
            bool isOwnBuffer);

    static int sPid;
    static std::string sRunTime;

    static void makeDirIfNotExist(const std::string& dir);
};
