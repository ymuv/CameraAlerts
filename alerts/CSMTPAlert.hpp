#pragma once
#include <chrono>
#include <mutex>

#ifdef __WITH_SMPT__
#include "alerts/smtp/CSMTPSender.hpp"
#endif


#include "alerts/IAlert.hpp"
#include "alerts/CAlertCoefConfig.hpp"
#include "alerts/CSMTPAlertConfig.hpp"

namespace NAlarms
{
struct CSMTPAlertConfig;

class CSMTPAlert: public IAlert
{
public:
    explicit CSMTPAlert(
            const CSMTPAlertConfig& config,
            const std::string& clientName,
            const std::string& url,
            const NAlarms::CAlertCoefConfig& coefConfig);

    virtual void onAlert(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo) override;

    virtual void checkAndSendForce(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds) override;

    virtual void checkAndSendReply(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds) override;

private:
    CSMTPAlertConfig mConfig;
    std::chrono::system_clock::rep mLastSendTime;

    std::string mLastAlgoName;
    bool mIsFirstTime; //true first config.mFirstTimeSkip seconds
    int mReplyId;
    std::string mPath; //if client - TCPClient_`name`

    static std::mutex sMutexUpdateForceSetTime;
    static std::chrono::system_clock::rep sLastForceSendTime;

//    std::string mTmpDir;

#ifdef __WITH_SMPT__
    NAlarms::CSMTPSender mSMTPSender;
#endif
};

}
