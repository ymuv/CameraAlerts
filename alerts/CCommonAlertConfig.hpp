#pragma once
#include <memory>

namespace NAlarms
{
struct CAlertSoundConfig;
struct CFinalVideoWriterConfig;
struct CTelegramAlertNotificationConfig;
struct CSMTPAlertConfig;

struct CCommonAlertConfig
{
    CCommonAlertConfig();
    std::shared_ptr<CAlertSoundConfig> mAlertSoundConfig;
    std::shared_ptr<CFinalVideoWriterConfig> mFinalVideoConfig;
    std::shared_ptr<CTelegramAlertNotificationConfig> mTelegramConfig;
    std::shared_ptr<CSMTPAlertConfig> mSMTPConfig;

    //QString mAlertName;
};

}
