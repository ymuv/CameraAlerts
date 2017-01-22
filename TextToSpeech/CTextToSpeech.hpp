#pragma once


#include <QObject>
#include <QString>
#include <QtTextToSpeech/QTextToSpeech>
#include <memory>



class QTextToSpeech;

namespace NTextToSpeach
{

struct CTextToSpeechConfig;

class CTextToSpeech : public QObject
{
    Q_OBJECT
public: //static
    static CTextToSpeech& getInstance(const CTextToSpeechConfig& config);
public:
    CTextToSpeech(const CTextToSpeechConfig& config);
    void say(const QString& str);

public slots:
    void stateChanged(QTextToSpeech::State state);

//    void say(const QString& str, const CTextToSpeechConfig& config);

private:
    const CTextToSpeechConfig& mConfig;
    std::shared_ptr<QTextToSpeech> mSpeech;
    QTextToSpeech::State mState;
};


}

