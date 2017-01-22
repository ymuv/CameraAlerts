#pragma once
#include <chrono>
#include <memory>

#include "alerts/IAlert.hpp"
#include "alerts/CTelegramAlertNotificationConfig.hpp"


namespace TgBot
{
    class Bot;
    class Message;
}

namespace NAlarms
{

class CTelegramAlertNotification : public IAlert
{
public: //static
    static std::shared_ptr<CTelegramAlertNotification> getInstance(const CTelegramAlertNotificationConfig& config);

public:
    virtual void onAlert(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo) override;

    bool isUserInVerifyList(std::shared_ptr<TgBot::Message> message) const;

    std::shared_ptr<TgBot::Bot> getBot() const;

    const CTelegramAlertNotificationConfig& getConfig() const;

private:
    explicit CTelegramAlertNotification(const CTelegramAlertNotificationConfig& config);

private:
    std::shared_ptr<TgBot::Bot> mBot;
    const CTelegramAlertNotificationConfig& mConfig;
    std::chrono::system_clock::rep mLastSendTime;
    bool mIsFirstTime;
};

}
