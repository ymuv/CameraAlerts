#ifdef __BUILD_WITH_QT_SOUND__
#include <QSound>
#endif

#include "alerts/CAlertSound.hpp"
#include "alerts/CAlertSoundConfig.hpp"

namespace NAlarms
{
CAlertSound::CAlertSound(const CAlertSoundConfig& config)
#ifdef __BUILD_WITH_QT_SOUND__
    : mSound(new QSound(config.mSound))
#endif
{
}

std::shared_ptr<CAlertSound> CAlertSound::getInstance(const CAlertSoundConfig& config)
{
    static std::shared_ptr<CAlertSound> instance(new CAlertSound(config));
    return instance;
}

void CAlertSound::onAlert(
        const CImageBuffer&,
        const std::vector<size_t>&,
        const std::string&,
        const std::string&)
{
#ifdef __BUILD_WITH_QT_SOUND__
    if (mSound)
    {
        mSound->play();
    }
#endif
}

void CAlertSound::checkAndSendForce(const CImageBuffer&, const std::vector<size_t>&)
{
}

void CAlertSound::checkAndSendReply(const CImageBuffer&, const std::vector<size_t>&)
{
}

}
