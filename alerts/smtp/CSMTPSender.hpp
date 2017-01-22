#pragma once

#include <vector>

#include "alerts/CSMTPAlertConfig.hpp"

namespace NAlarms
{
enum class SMTPResult
{
    SMTP_SEND_OK = 0,
    SMTP_CONNECT_FAIL,
    SMTP_LOGIN_FAIL,
    SMTP_SEND_FAIL,
    SMTP_NO_DATA,
};
class CSMTPSender
{
public:
    explicit CSMTPSender(const std::string& runDateTime);
    static bool isConfigFull(const CSMTPAlertConfig& smtpConfig);

    SMTPResult sendEmail(
            const CSMTPAlertConfig& smtpConfig,
            const std::vector<std::string>& fileNames,
            const std::string& alertName,
            const std::string& dateTime,
            const std::string& subjectInfoStr,
            const std::string& infoStr,
            std::string& errorString) const;
private:
    const std::string mRunDateTime;

};
}
