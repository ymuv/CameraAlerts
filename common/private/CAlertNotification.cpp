#include <iterator>

#include "Image/CImageBuffer.hpp"
#include "alerts/CCommonAlert.hpp"
#include "common/CAlertNotification.hpp"
#include "common/CConfig.hpp"

using namespace std;


namespace
{

template <class T>
std::string vecToStr(std::vector<T> vec)
{
    std::ostringstream oss;

    if (!vec.empty())
    {
      // Convert all but the last element to avoid a trailing ","
      std::copy(vec.begin(), vec.end()-1,
          std::ostream_iterator<T>(oss, ","));

      // Now add the last element with no delimiter
      oss << vec.back();
    }
    return oss.str();
}
}

CAlertNotification::CAlertNotification(size_t bufferId)
    : CAlertNotification(std::to_string(bufferId), "")
{
}

CAlertNotification::CAlertNotification(
        const std::vector<size_t>& bufferIds,
        const NAlarms::CAlertCoefConfig& coefConfig)
    : CAlertNotification(vecToStr(bufferIds), coefConfig)
{
}

CAlertNotification::CAlertNotification(
        const std::string& infoStr,
        const NAlarms::CAlertCoefConfig& coefConfig)
    : CAlertNotification( infoStr, "", coefConfig)
{

}

CAlertNotification::CAlertNotification(
        const std::string& clientName,
        const std::string& url,
        const NAlarms::CAlertCoefConfig& coefConfig)
    : mNewAlertPtr(new NAlarms::CCommonAlert(
                       *CConfig::getInstance().mCommonAlertConfig, clientName, url, coefConfig))
{
}

bool CAlertNotification::sendAlert(
        const CImageBuffer& buff,
        const std::vector<size_t>& bufferIds,
        const std::string& alertAlgorithm,
        const std::string& alertInfo,
        NAlarms::ALERT_TYPE type)
{
    if (type == NAlarms::ALERT_TYPE::CONDITION_WARNING)
    {
        mNewAlertPtr->onWarning(buff,bufferIds, alertAlgorithm,alertInfo);
    }
    else if (type == NAlarms::ALERT_TYPE::CONDITION_ALERT)
    {
        mNewAlertPtr->onAlert(buff,bufferIds, alertAlgorithm,alertInfo);
//#ifdef __WITH_SMPT__
//        if ((diffTime > mConfig.mMinIntervalRepeatSend)
//                && !mIsFirstTime)
//        {
//            mLastSendTime = CDateTime::getSecondsSinceEpoch();
//            mLastAlgoName = alertAlgorithm;

//            mReplyId = 0;

//            if (mConfig.mIsSendAsync)
//            {
//                std::vector<matPtrLists> beffersCopy;
//                std::vector<matPtrLists> beffersLongCopy;

//                for (const auto bufferId : bufferIds)
//                {
//                    if (buff.isbufferNotExistAndNotFull(bufferId))
//                    {
//                        continue;
//                    }
//                    beffersCopy.push_back(buff.getCopy(bufferId));
//                    beffersLongCopy.push_back(buff.getLongCopy(bufferId));
//                }
//                std::thread thread(
//                            saveAndSendEmailManyBuffers,
//                            alertAlgorithm,
//                            alertInfo,
//                            beffersCopy,
//                            beffersLongCopy,
//                            bufferIds,
//                            true,
//                            std::cref(mConfig),
//                            std::cref(mSMTPSender),
//                            matMask);
//                thread.detach();
//            }
//            else
//            {
//                std::vector<matPtrLists> beffersCopy;
//                std::vector<matPtrLists> beffersLongCopy;
//                for (const auto bufferId : bufferIds)
//                {
//                    beffersCopy.push_back(buff.getCopy(bufferId));
//                    beffersLongCopy.push_back(buff.getLongCopy(bufferId));
//                }
//                result = saveAndSendEmailManyBuffers(
//                            alertAlgorithm,
//                            alertInfo,
//                            beffersCopy,
//                            beffersLongCopy,
//                            bufferIds,
//                            true,
//                            mConfig,
//                            mSMTPSender,
//                            matMask);
//            }
//        }
//#endif
//        if (!mConfig.mRunExternalCommandsOnAlert.empty())
//        {
//            std::string str = mConfig.mRunExternalCommandsOnAlert
//                    + " \""  + alertAlgorithm + "__" + std::to_string(bufferIds[0])
//                    + buff.getBufferInfo(bufferIds[0])
//                    + "\" &";
//            (void) system(str.c_str());
//        }
//        return result;
    }
    return false;

}

void CAlertNotification::checkAndSendReplyAndForce(
        const CImageBuffer& buff, const std::vector<size_t>& bufferIds)
{
    checkAndSendForce(buff, bufferIds);
    checkAndSendReply(buff, bufferIds);
}

bool CAlertNotification::checkAndSendForce(const CImageBuffer& buff, const std::vector<size_t>& bufferIds)
{
    mNewAlertPtr->checkAndSendForce(buff,bufferIds);
    return false;
}

bool CAlertNotification::checkAndSendReply(const CImageBuffer& buff, const std::vector<size_t>& bufferIds)
{
    mNewAlertPtr->checkAndSendReply(buff, bufferIds);
    return false;
}

//void CAlertNotification::saveVideoToVideoPath(
//        const CImageBuffer& buff,
//        size_t bufferId,
//        const cv::Mat& mask,
//        const std::string& alertAlgo,
//        const std::string& alertInfo)
//{
//    if (buff.isbufferNotExistAndNotFull(bufferId))
//    {
//        return;
//    }

//    if (!mConfig.mSaveVideoPath.empty())
//    {
//       auto matPtr =  buff.getLast(bufferId);
//       CMatWithTimeStampLocker matLocker(matPtr);
//       const std::string& date =matPtr->getDate();
//        createDirs(date);
//        static const std::string fileFormat = ".avi";
//        std::string videoFileName =
//                mConfig.mSaveVideoPath + "/"
//                + date + "/"
//                + std::to_string(bufferId) + "_"
//                + matPtr->getDateTimeWithMS()
//                + "_" + std::to_string(buff.getLast(bufferId)->getId())
//                + "_" + alertAlgo
//                + fileFormat;

//        std::string infoStr = "last id: " + std::to_string(matPtr->getId());
//        if (buff.size(bufferId) > 2)
//        {
//            infoStr += " prev id: " + std::to_string(buff.getLast2(bufferId)->getId());
//        }

//        CVideoWriter::writeVideo(
//                    videoFileName,
//                    buff,
//                    bufferId,
//                    mask,
//                    alertAlgo + "\n" + infoStr + "\n" + alertInfo,
//                    mConfig,
//                    true,
//                    mConfig.mIsSaveVideoAsync);
//    }
//}

