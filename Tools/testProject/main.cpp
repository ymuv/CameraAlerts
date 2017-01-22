#include <QCoreApplication>

#if 1

int main(int argc, char *argv[])
{

}
#else

#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <memory>
#include "logging/Log.hpp"

#include <QDateTime>
#include <QtTextToSpeech/QTextToSpeech>

using namespace std;
using namespace cv;

#include "conditions/CCondition.hpp"
QVector<QVoice> m_voices;
QTextToSpeech *m_speech;

void f(std::shared_ptr<int>& ptr)
{
   std::shared_ptr<int> newPtr(new int);
   ptr =  newPtr;
}

int main(int argc, char *argv[])
{
     QCoreApplication app(argc, argv);

    m_speech = new QTextToSpeech();

    foreach (QString engine, QTextToSpeech::availableEngines())
    {
       LOG_WARN << engine;

    }

    m_voices = m_speech->availableVoices();
    m_speech->setVoice(m_voices[0]);
    LOG_WARN << "size" << m_voices.size() << m_voices[0].gender();

    for (const auto& voice : m_voices)
    {
        LOG_WARN << "gend" << voice.gender();
        m_speech->setVoice(voice);
        m_speech->setVolume(100);
//        m_speech->setPitch(0);
        m_speech->say("AAAA");


        m_speech->say("OOO Hello worlsd");
        m_speech->say("OOO Hello worlsd");
        m_speech->say("OOO Hello worlsd");
    }



    LOG_WARN << m_speech->state();
    LOG_WARN << m_speech->state();
    LOG_WARN << m_speech->state();
    LOG_WARN << m_speech->state();
    LOG_WARN << m_speech->state();

    return app.exec();
}
#endif
