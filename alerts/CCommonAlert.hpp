#pragma once

#include "IAlert.hpp"
#include "CAlertCoefConfig.hpp"

namespace NAlarms
{
struct CCommonAlertConfig;
class CCommonAlert : public IAlert
{
public:
    CCommonAlert(
            const CCommonAlertConfig& commonAlertConfig,
            const std::string& clientName,
            const std::string& url,
            const CAlertCoefConfig& config);

    virtual void onAlert(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo) override;

    virtual void onWarning(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo) override;

    virtual void checkAndSendForce(const CImageBuffer& buff, const std::vector<size_t>& bufferIds) override;
    virtual void checkAndSendReply(const CImageBuffer& buff, const std::vector<size_t>& bufferIds) override;

    std::vector<IAlert::Ptr> mAlertList;
    virtual ~CCommonAlert();


};
}

