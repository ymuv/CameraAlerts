#include "alerts/IAlert.hpp"

namespace NAlarms
{

void IAlert::checkAll(
        const CImageBuffer& buff,
        const std::vector<size_t>& bufferIds,
        NAlarms::ALERT_TYPE condType,
        const std::string& infoStr1,
        const std::string& infoStr2)
{
    if (condType == NAlarms::ALERT_TYPE::CONDITION_ALERT)
    {
        onAlert(buff, bufferIds, infoStr1, infoStr2);
    }
    else if (condType == NAlarms::ALERT_TYPE::CONDITION_WARNING)
    {
        onWarning(buff, bufferIds, infoStr1, infoStr2);
    }
    else if (condType == NAlarms::ALERT_TYPE::NO_CONDITION)
    {
        checkAndSendReply(buff, bufferIds);
        checkAndSendForce(buff, bufferIds);
    }
}

}
