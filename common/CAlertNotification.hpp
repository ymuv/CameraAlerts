#pragma once

#include <memory>

#include "alerts/CAlertCoefConfig.hpp"
#include "alerts/IAlert.hpp"

class CImageBuffer;

class CAlertNotification
{
public:
    CAlertNotification(size_t bufferId);
    CAlertNotification(
            const std::vector<size_t>& bufferIds,
            const NAlarms::CAlertCoefConfig& coefConfig = NAlarms::CAlertCoefConfig());

    CAlertNotification(
            const std::string& infoStr,
            const NAlarms::CAlertCoefConfig& coefConfig = NAlarms::CAlertCoefConfig());

    CAlertNotification(
            const std::string& clientName,
            const std::string& url,
            const NAlarms::CAlertCoefConfig& coefConfig = NAlarms::CAlertCoefConfig());

    bool sendAlert(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo,
            NAlarms::ALERT_TYPE type);

    void checkAndSendReplyAndForce(const CImageBuffer& buff, const std::vector<size_t>& bufferIds);
    bool checkAndSendForce(const CImageBuffer& buff, const std::vector<size_t>& bufferIds);
    bool checkAndSendReply(const CImageBuffer& buff, const std::vector<size_t>& bufferIds);

//private: //methods
//    void saveVideoToVideoPath(
//            const CImageBuffer& buff,
//            size_t bufferId,
//            const cv::Mat&mask,
//            const std::string& alertAlgo,
//            const std::string& alertInfo);

private: //fields
    NAlarms::IAlert::Ptr mNewAlertPtr;
};
