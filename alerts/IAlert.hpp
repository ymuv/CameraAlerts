#pragma once

#include <opencv2/opencv.hpp>
#include <memory>
#include <string>
#include <vector>

class CImageBuffer;

namespace NAlarms
{

enum class ALERT_TYPE
{
    NO_CONDITION = 0,
    CONDITION_WARNING = 1,
    CONDITION_ALERT = 2,
    CONDITION_FATAL = 3,
};

class IAlert
{
public:
    using Ptr = std::shared_ptr<IAlert>;

public:
    virtual void onAlert(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo) = 0;

    virtual void onWarning(
            const CImageBuffer&,
            const std::vector<size_t>&,
            const std::string&,
            const std::string&)
    {
    }

    virtual void checkAndSendForce(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds)
    {
    }

    virtual void checkAndSendReply(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds)
    {
    }

    virtual void checkAll(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            NAlarms::ALERT_TYPE condType,
            const std::string& infoStr1 = "",
            const std::string& infoStr2 = "");

    virtual ~IAlert(){}
};
}
