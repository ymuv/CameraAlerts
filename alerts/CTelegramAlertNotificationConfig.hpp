#pragma once

#include <string>
#include <vector>

#include "CAlertConfig.hpp"

namespace NAlarms
{

struct CTelegramAlertNotificationConfig : CAlertConfig
{
    std::string mToken;
    std::vector<int64_t> mVerifyUsers;
    std::vector<int64_t> mAlertUsers;
    bool mIsSendImagesWhenMaskSendet;
};
}
