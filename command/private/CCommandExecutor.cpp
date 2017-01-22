#include "command/CCommand.hpp"
#include "command/CCommandExecutor.hpp"
#include "common/CConfig.hpp"

#ifdef __WITH_TEXT_TO_SPEECH__
#include "TextToSpeech/CTextToSpeech.hpp"
#endif

namespace NCommand
{

CCommandExecutor&CCommandExecutor::getInstance()
{
    static CCommandExecutor instance;
    return instance;
}

void CCommandExecutor::doCommand(const CCommand& command, const QString& args)
{
#ifdef __WITH_TEXT_TO_SPEECH__
    if (!command.mSay.isEmpty())
    {
        NTextToSpeach::CTextToSpeech& speech = NTextToSpeach::CTextToSpeech::getInstance(*CConfig::getInstance().mSpeechConfig);
        speech.say(command.mSay + " " + args);
    }
#endif

}

CCommandExecutor::CCommandExecutor()
{

}

}
