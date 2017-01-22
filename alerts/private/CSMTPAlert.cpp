#include <QDir>
#include <string>
#include <thread>

#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "alerts/CSMTPAlertConfig.hpp"
#include "alerts/CSMTPAlert.hpp"
#include "alerts/private/CVideoWriter.hpp"
#include "alerts/zlipWrapper/CZipCreator.hpp"
#include "common/CAlgoConfig.hpp"
#include "common/CConfig.hpp"
#include "common/CDateTime.hpp"
#include "common/NTypes.hpp"
#include "common/CMainFunctions.hpp"
#include "common/CMemoryUssage.hpp"
#include "logging/Log.hpp"

std::mutex NAlarms::CSMTPAlert::sMutexUpdateForceSetTime;
std::chrono::system_clock::rep NAlarms::CSMTPAlert::sLastForceSendTime;

namespace
{

const static bool sUseThread = true;

#ifdef __WITH_SMPT__
void addFileToSendList(
        const std::vector<std::string>& inFilesList,
        std::vector<std::string>& outFilesList,
        size_t fileId)
{
    if (inFilesList.size() > fileId)
    {
        const static std::string videoPrefix = ".avi";
        if (inFilesList[inFilesList.size() -fileId].find(videoPrefix)
                == std::string::npos)
        {
            outFilesList.push_back(inFilesList[inFilesList.size() - fileId]);
        }
    }
}

std::string runExternalCommandAndGetOutput(const NAlarms::CSMTPAlertConfig& config)
{
#ifdef __linux__
    if (!config.mRunExternalCommands.empty())
    {
        char buffer[128];
        std::string result = "";
        std::shared_ptr<FILE> pipe(
                    popen(config.mRunExternalCommands.c_str(),
                          "r"), pclose);
        if (!pipe)
        {
            LOG_WARN << "popen() failed!";
            return "";
        }
        while (!feof(pipe.get()))
        {
            if (fgets(buffer, 128, pipe.get()) != NULL)
                result += buffer;
        }
        return result;
    }
#endif
    return "";
}

void sendEmailAndDeleteFiles(
        const std::string& alertAlgorithm,
        const std::string& lastDateTime,
        const std::string& alertInfo,
        const std::string& imageSize,
        const std::string& tmpIdDir,
        size_t bufferId,
        NTypes::tMatIdType lastImgId,
        NTypes::tMatIdType prevImgId,
        const std::vector<std::string>& savedFiles,
        const NAlarms::CSMTPSender& sender,
        const NAlarms::CSMTPAlertConfig& config,
        bool isSendFull)
{
    std::string archName = tmpIdDir
            + "/"
            + std::to_string(bufferId) + "_"
            + lastDateTime +
            + "__" + std::to_string(lastImgId) + "__"
            + alertAlgorithm + ".zip";

    if (!NZIP::CZipCreator::createZipFile(
                archName,
                config.getZipPassword(),
                savedFiles))
    {
        return;
    }

    std::vector<std::string> files({archName});
    if (isSendFull && config.mLastImagesCountToEmailEncrypted > 0)
    {
        for (size_t i = 0;
             i<  std::min(config.mLastImagesCountToEmailEncrypted, savedFiles.size());
             i++)
        {
            addFileToSendList(savedFiles, files,i + 1); //+1 because skip video
        }
    }

    std::string externalCommandOutput = runExternalCommandAndGetOutput(config);

    std::stringstream infoStringStream;
    infoStringStream
            << "alert in " << alertAlgorithm
            << " on " << lastDateTime << "(buffer:" << bufferId << ")\n\n"
            << "run Time" << CMainFunctions::sRunTime << "\n"
            << "last image id: " << lastImgId
            << " prev image id: " << prevImgId << "\n"
            << alertInfo << "\n\n"
            << "bufferId: " << bufferId
            << "; mem ussage: " << CMemoryUssage::processMemoryUssage()
            << "; memMax ussage: " << CMemoryUssage::processMaxMemoryUssage()
            << "; image size: " << imageSize
            << "; buffer size:" << CImageBuffer::getBufferInstance().size(bufferId)
            << "\n\n"  << CConfig::getInstance()
            << "\n\n" << CAlgoConfig::getInstance()
            << "\n\n" << externalCommandOutput;

    std::string subjectInfo = "buff" + std::to_string(bufferId);

    const static size_t  MAX_REPLY_COUNT = 1;

    std::string errorStr;

    for (size_t i = 0; i < MAX_REPLY_COUNT; i++)
    {
        NAlarms::SMTPResult result = sender.sendEmail(
                    config,
                    files,
                    alertAlgorithm,
                    lastDateTime,
                    subjectInfo,
                    infoStringStream.str(),
                    errorStr);
        if (result == NAlarms::SMTPResult::SMTP_SEND_OK)
        {
            break;
        }
    }
    if (!errorStr.empty())
    {
        LOG_WARN << "fail send multiple times(" <<MAX_REPLY_COUNT << "times; last error " << errorStr
                 << "buffer id " << bufferId;
    }

    QFile(archName.c_str()).remove();
    for (const auto& file: savedFiles)
    {
        QFile(file.c_str()).remove();
    }
    QDir(tmpIdDir.c_str()).rmdir(tmpIdDir.c_str());
}

bool callScriptStatic(
        const std::string& alertAlgorithm,
        const std::string& alertInfo,
        const std::string& tmpIdDir,
        size_t bufferId,
        bool isSendFull,
        const std::vector<std::string>& savedNames,
        const pMatWithTimeStamp lastMat,
        NTypes::tMatIdType prevImgId,
        const NAlarms::CSMTPSender& SMTPSender,
        const std::string tmpDir,
        const NAlarms::CSMTPAlertConfig& config)
{
    if (savedNames.empty())
    {
        return false;
    }

    if (!lastMat)
    {
        LOG_WARN << "mat is empty";
        return false;
    }

    if (CDateTime::getCurrentHour() >= config.mSendFrom
            && CDateTime::getCurrentHour() < config.mSendTo
            && isSendFull)
    {
        isSendFull = true;
    }
    else
    {
        isSendFull = false;
    }

    if (!tmpDir.empty()
            && SMTPSender.isConfigFull(config))
    {
        std::string imageSizeString = std::to_string(lastMat->getMat().size().width)
                + "x"+ std::to_string(lastMat->getMat().size().height);
        sendEmailAndDeleteFiles(
                    alertAlgorithm,
                    lastMat->getDateTimeWithMS(),
                    alertInfo,
                    imageSizeString,
                    tmpIdDir,
                    bufferId,
                    lastMat->getId(),
                    prevImgId,
                    savedNames,
                    SMTPSender,
                    config,
                    isSendFull);
    }
    return true;
}


pMatWithTimeStamp addFilesAndSaveStatic(
        const CImageBuffer& bufferCopy,
        const std::string& alertAlgo,
        const std::string& alertInfo,
        std::vector<size_t> bufferIds,
        std::string& tmpIdDir,
        std::vector<std::string> &savedNames,
        NTypes::tMatIdType& prevImgId,
        const NAlarms::CSMTPAlertConfig& config,
        const std::string tmpDir)
{
    pMatWithTimeStamp returnMat;
    for (size_t buffId = 0; buffId < bufferIds.size(); buffId++)
    {
        if (!bufferCopy.isBufferExistAndNotEmpty(buffId))
        {
            continue;
        }
        const matPtrLists& buff = bufferCopy.getList(buffId);

        size_t bufferId = bufferIds[buffId];

        auto size = buff.size();
        if (size == 0)
        {
            continue;
        }
        returnMat = bufferCopy.getLast(buffId);

        if (returnMat)
        {
            tmpIdDir = tmpDir + "/" + std::to_string(returnMat->getId()) + "/";
            CMainFunctions::makeDirIfNotExist(tmpIdDir);

            if (config.mLastImagesCountToEmailInZip > 0)
            {
                long imageId = size - config.mLastImagesCountToEmailInZip - 1;
                if (imageId < 0)
                {
                    imageId = 0;
                }

                for (; imageId < size; imageId++)
                {
                    const auto matPtr = buff.at(imageId);
                    std::string savedName = matPtr->saveTo(tmpIdDir, buffId);
                    if (!savedName.empty())
                    {
                        savedNames.push_back(savedName);
                    }
                }
            }

            if (config.mIsWriteVideoToEmail)
            {
                static const std::string fileFormat = ".avi";
                std::string videoFileName =
                        tmpIdDir
                        + "/" + std::to_string(bufferId) + "_"
                        + returnMat->getDateTimeWithMS()
                        + "_" + std::to_string(returnMat->getId())
                        + "_" + alertAlgo
                        + fileFormat;
                std::string infoStr = "last id: " + std::to_string(returnMat->getId());
                if (buff.size() > 2)
                {
                    infoStr += " prev id: " + std::to_string(buff[size - 2]->getId());
                    prevImgId = buff[size - 2]->getId();
                }

                if (CVideoWriter::writeVideo(
                            videoFileName,
                            bufferCopy,
                            buffId,
                            alertAlgo + "\n" + infoStr + "\n" + alertInfo,
                            config))
                {
                    savedNames.push_back(videoFileName);
                }
            }

            if (returnMat->hasMask())
            {
                const cv::Mat mask = returnMat->getMask();
                if (!mask.empty())
                {
                    std::string maskName = tmpIdDir + "/" + returnMat->getDateTimeWithMS() + "__" +
                            std::to_string(returnMat->getId()) + "_mask.jpg";
                    if (cv::imwrite(maskName, mask))
                    {
                        savedNames.push_back(maskName);
                    }
                }
            }
        }
    }
    return returnMat;
}


bool saveAndSendEmailManyBuffers(
        const CImageBuffer& bufferCopy,
        const std::string& alertAlgorithm,
        const std::string& alertInfo,
        const std::vector<size_t>& bufferIds,
        bool isSendFull,
        const NAlarms::CSMTPAlertConfig& config,
        const NAlarms::CSMTPSender& SMTPSender)
{
    std::vector<std::string> savedNames;
    if (!config.mTmpDir.empty()
            && SMTPSender.isConfigFull(config)
            && !bufferIds.empty())
    {
        std::string tmpIdDir;
        pMatWithTimeStamp lastMat;
        NTypes::tMatIdType prevImgId = 0;
        lastMat = addFilesAndSaveStatic(
                    bufferCopy,
                    alertAlgorithm,
                    alertInfo,
                    bufferIds,
                    tmpIdDir,
                    savedNames,
                    prevImgId,
                    config,
                    config.mTmpDir);

        CMatWithTimeStampLocker locker(lastMat);
        bool result = callScriptStatic(
                    alertAlgorithm,
                    alertInfo,
                    tmpIdDir,
                    bufferIds[0],
                    isSendFull,
                    savedNames,
                    lastMat,
                    prevImgId,
                    SMTPSender,
                    config.mTmpDir,
                    config);
        return result;
    }
    else
    {
        return false;
    }
}

#endif
}


NAlarms::CSMTPAlert::CSMTPAlert(
        const NAlarms::CSMTPAlertConfig& config,
        const std::string& clientName,
        const std::string& url,
        const NAlarms::CAlertCoefConfig& coefConfig)
    : mConfig(config)
    , mIsFirstTime(true)
    , mReplyId(0)
#ifdef __WITH_SMPT__
    , mSMTPSender(CDateTime::getDateTimeAsString())
#endif
{
    mConfig.mMinIntervalRepeatSend *= coefConfig.mAlertMinIntervalRepeatSendCoef;
    mConfig.mReplyCount *= coefConfig.mAlertReplyCountCoef;

    if (!clientName.empty())
    {
        mPath = clientName;
        if (!url.empty())
        {
            mPath += "_" + url;
        }
    }

    if (!mConfig.mTmpDir.empty())
    {
        if (!clientName.empty() || url.empty())
        {
            mConfig.mTmpDir = mConfig.mTmpDir + "/" + mPath + "/";
//            mTmpDir
        }
    }
    mLastSendTime = CDateTime::getSecondsSinceEpoch();
    std::lock_guard<std::mutex> lockForce(sMutexUpdateForceSetTime);
    sLastForceSendTime = CDateTime::getSecondsSinceEpoch();

}

void NAlarms::CSMTPAlert::onAlert(
        const CImageBuffer& buff,
        const std::vector<size_t>& bufferIds,
        const std::string& alertAlgorithm,
        const std::string& alertInfo)
{
#ifdef __WITH_SMPT__
    auto diffTime = CDateTime::getSecondsSinceEpoch() - mLastSendTime;
    if (mIsFirstTime && (diffTime > mConfig.mFirstTimeSkip))
    {
        mIsFirstTime = false;
    }
    if ((diffTime > mConfig.mMinIntervalRepeatSend) && !mIsFirstTime)
    {
        mLastSendTime = CDateTime::getSecondsSinceEpoch();
        mLastAlgoName = alertAlgorithm;

        mReplyId = 0;

        if (sUseThread)
        {
            CImageBuffer bufferCopy = buff;
            std::thread thread(
                        saveAndSendEmailManyBuffers,
                        bufferCopy,
                        alertAlgorithm,
                        alertInfo,
                        bufferIds,
                        true,
                        std::cref(mConfig),
                        std::cref(mSMTPSender));
                thread.detach();
            }
            else
            {
                saveAndSendEmailManyBuffers(
                            buff,
                            alertAlgorithm,
                            alertInfo,
                            bufferIds,
                            true,
                            mConfig,
                            mSMTPSender);
            }
        }
#endif
}

void NAlarms::CSMTPAlert::checkAndSendForce(
        const CImageBuffer& buff, const std::vector<size_t>& bufferIds)
{
#ifdef __WITH_SMPT__
    auto currentTime = CDateTime::getSecondsSinceEpoch();
    if ((currentTime - sLastForceSendTime) > mConfig.mForceSendInterval
            && (mConfig.mForceSendInterval > 0))
    {
        std::unique_lock<std::mutex> lockForce(sMutexUpdateForceSetTime, std::try_to_lock);
        if(lockForce.owns_lock())
        {
            sLastForceSendTime = currentTime - mConfig.mMinIntervalRepeatSend;

            sLastForceSendTime = CDateTime::getSecondsSinceEpoch();
            std::string info = "FORCE_SEND";

            if (sUseThread)
            {
                std::thread thread(
                            saveAndSendEmailManyBuffers,
                            buff,
                            info,
                            "",
                            bufferIds,
                            true,
                            std::cref(mConfig),
                            std::cref(mSMTPSender));
                    thread.detach();
                }
                else
                {
                    saveAndSendEmailManyBuffers(
                                buff,
                                info,
                                "",
                                bufferIds,
                                true,
                                mConfig,
                                mSMTPSender);
                }
        }
    }
#endif
}

void NAlarms::CSMTPAlert::checkAndSendReply(
        const CImageBuffer& buff, const std::vector<size_t>& bufferIds)
{
#ifdef __WITH_SMPT__
    auto currentTime = CDateTime::getSecondsSinceEpoch();

    if (!mLastAlgoName.empty()
            && mConfig.mReplyDiffTime > 0
            && mConfig.mReplyCount > 0
            && mReplyId < mConfig.mReplyCount
            && (currentTime - mLastSendTime) > (mConfig.mReplyDiffTime * (mReplyId + 1)))
    {

        std::string info = "REPLY";
        std::string subject = "REPLY_" + std::to_string(mReplyId) + "_" + mLastAlgoName;
        mReplyId++;

        if (sUseThread)
        {
            CImageBuffer bufferCopy = buff;
            std::thread thread(
                        saveAndSendEmailManyBuffers,
                        bufferCopy,
                        subject,
                        info,
                        bufferIds,
                        true,
                        std::cref(mConfig),
                        std::cref(mSMTPSender));
            thread.detach();
        }
        else
        {
            saveAndSendEmailManyBuffers(
                        buff,
                        subject,
                        info,
                        bufferIds,
                        true,
                        mConfig,
                        mSMTPSender);
        }
    }
#endif

}
