#include "Image/CImageBuffer.hpp"
#include "alerts/CAlertSound.hpp"
#include "alerts/CAlertSoundConfig.hpp"
#include "alerts/CCommonAlert.hpp"
#include "alerts/CCommonAlertConfig.hpp"
#include "alerts/CFinalVideoWriter.hpp"
#include "alerts/CFinalVideoWriterConfig.hpp"
#include "alerts/CTelegramAlertNotification.hpp"
#include "alerts/CSMTPAlert.hpp"

#ifdef __WITH_SMPT__
#include "alerts/smtp/CSMTPSender.hpp"
#endif

namespace NAlarms
{

CCommonAlert::CCommonAlert(
        const CCommonAlertConfig& commonAlertConfig,
        const std::string& clientName,
        const std::string& url,
        const CAlertCoefConfig& config)
{
    if (commonAlertConfig.mTelegramConfig &&
            !commonAlertConfig.mTelegramConfig->mToken.empty())
    {
        mAlertList.push_back(IAlert::Ptr(CTelegramAlertNotification::getInstance(
                                             *commonAlertConfig.mTelegramConfig)));
    }
    if (commonAlertConfig.mAlertSoundConfig && !commonAlertConfig.mAlertSoundConfig->mSound.isEmpty())
    {
        mAlertList.push_back(CAlertSound::getInstance(*commonAlertConfig.mAlertSoundConfig));
    }
    if (commonAlertConfig.mFinalVideoConfig && !commonAlertConfig.mFinalVideoConfig->mSaveFinalVideoPaths.empty())
    {
        mAlertList.push_back(CFinalVideoWriter::getInstance(*commonAlertConfig.mFinalVideoConfig));
    }

#ifdef __WITH_SMPT__
    if (commonAlertConfig.mSMTPConfig && CSMTPSender::isConfigFull(*commonAlertConfig.mSMTPConfig))
    {
        mAlertList.push_back(std::make_shared<CSMTPAlert>(
                                 *commonAlertConfig.mSMTPConfig,
                                 clientName,
                                 url,
                                 config));
    }
#endif
}

void CCommonAlert::onAlert(const CImageBuffer& buff, const std::vector<size_t>& bufferIds, const std::string& alertAlgorithm, const std::string& alertInfo)
{
    const CImageBuffer bufferCopy = buff;
    for (const auto& alertPtr : mAlertList)
    {
        alertPtr->onAlert(bufferCopy, bufferIds, alertAlgorithm, alertInfo);
    }
}

void CCommonAlert::onWarning(
        const CImageBuffer& buff,
        const std::vector<size_t>& bufferIds,
        const std::string& alertAlgorithm,
        const std::string& alertInfo)
{
    for (const auto& alertPtr : mAlertList)
    {
        alertPtr->onWarning(buff, bufferIds, alertAlgorithm, alertInfo);
    }
}

void CCommonAlert::checkAndSendForce(const CImageBuffer& buff, const std::vector<size_t>& bufferIds)
{
    for (const auto& alertPtr : mAlertList)
    {
        alertPtr->checkAndSendForce(buff, bufferIds);
    }
}

void CCommonAlert::checkAndSendReply(const CImageBuffer& buff, const std::vector<size_t>& bufferIds)
{
    for (const auto& alertPtr : mAlertList)
    {
        alertPtr->checkAndSendReply(buff, bufferIds);
    }
}

CCommonAlert::~CCommonAlert()
{

}
}
