#pragma once

#include "alerts/IAlert.hpp"

class QSound;

namespace NAlarms
{
struct CAlertSoundConfig;

class CAlertSound : public IAlert
{
public: //static methods
    static std::shared_ptr<CAlertSound> getInstance(const CAlertSoundConfig& config);

public: //methods
    virtual void onAlert(
            const CImageBuffer&,
            const std::vector<size_t>&,
            const std::string&,
            const std::string&) override;

    virtual void checkAndSendForce(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds) override;

    virtual void checkAndSendReply(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds) override;

private: //methods
    explicit CAlertSound(const CAlertSoundConfig& config);

private: //field
    std::unique_ptr<QSound> mSound;
};
}
