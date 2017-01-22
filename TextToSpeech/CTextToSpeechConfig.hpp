#pragma once
#include <QString>

namespace NTextToSpeach
{
struct CTextToSpeechConfig
{
    int mVolume;
    int mPitch;
    int mRate;
    QString mEngine;

    int mLocaleId = -1;
    int mLangId = -1;
    int mVoiceId = -1;
};
}
