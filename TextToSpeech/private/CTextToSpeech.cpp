#include <QtTextToSpeech/QTextToSpeech>
#include <QCoreApplication>
#include <QVector>

#include "TextToSpeech/CTextToSpeech.hpp"
#include "TextToSpeech/CTextToSpeechConfig.hpp"
#include "logging/Log.hpp"

namespace NTextToSpeach
{

CTextToSpeech&CTextToSpeech::getInstance(const CTextToSpeechConfig& config)
{
    static CTextToSpeech instance(config);
    return instance;
}

CTextToSpeech::CTextToSpeech(const CTextToSpeechConfig& config)
    : mConfig(config)
    , mState(QTextToSpeech::Ready)
{
    if (mConfig.mEngine.isEmpty())
    {
        mSpeech.reset(new QTextToSpeech);
    }
    else
    {
        mSpeech.reset(new QTextToSpeech(mConfig.mEngine));
    }

    connect(mSpeech.get(), &QTextToSpeech::stateChanged, this, &CTextToSpeech::stateChanged);

    {
        if (mConfig.mLocaleId >= 0)
        {
            QVector<QLocale> locales = mSpeech->availableLocales();
            if (mConfig.mLocaleId < locales.size())
            {
                mSpeech->setLocale(locales[mConfig.mLocaleId]);
            }
        }

        if (mConfig.mVoiceId >= 0)
        {
            QVector<QVoice> voices = mSpeech->availableVoices();
            if (mConfig.mVoiceId < voices.size())
            {
                mSpeech->setVoice(voices[mConfig.mVoiceId]);
            }
        }
    }
}

void CTextToSpeech::say(const QString& str)
{
    if (mState != QTextToSpeech::Speaking)
    {
        mState = QTextToSpeech::Speaking;
        mSpeech->say(str);
    }
}

void CTextToSpeech::stateChanged(QTextToSpeech::State state)
{
    mState = state;
}
}
