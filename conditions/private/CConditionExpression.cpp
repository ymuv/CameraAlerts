#include <QJSEngine>

#include "Image/CImageBuffer.hpp"
#include "Text/CTextBuffer.hpp"
#include "conditions/CConditionExpression.hpp"
#include "conditions/CCondition.hpp"
#include "command/CCommand.hpp"
#include "command/CCommandExecutor.hpp"
#include "common/CConfig.hpp"
#include "common/CDateTime.hpp"
#include "logging/Log.hpp"

namespace NCondition
{

CConditionExpression::CConditionExpression()
    : mEngine(new QJSEngine)
{
}

void CConditionExpression::doCondition(const NCondition::CCondition& condition, const QString& args)
{
    if (!condition.mCommand.isEmpty())
    {
        const NCommand::CCommandsMap& commands = *CConfig::getInstance().mCommandMap;
        if (commands.mCommandMap.contains(condition.mCommand))
        {
            NCommand::CCommandExecutor::getInstance().doCommand(commands.mCommandMap[condition.mCommand], args);
        }
        else
        {
            LOG_WARN << "Command of expression not fouund in command list";
        }
    }
}

void CConditionExpression::setProperty(const QString& property, const QJSValue& value)
{
    if (mEngine->globalObject().hasProperty(property))
    {
        const auto prevValue = mEngine->globalObject().property(property);
        mEngine->globalObject().setProperty("PREV_" + property, prevValue);
    }
    mEngine->globalObject().setProperty(property, value);


}

CConditionExpression::Ptr CConditionExpression::getInstance()
{
    static Ptr ptr(new CConditionExpression);
    return ptr;
}

void CConditionExpression::checkAllExpression()
{
    std::unique_lock<std::mutex> lockForce(mMutex, std::try_to_lock);
    if(lockForce.owns_lock())
    {
        for (const auto& pattern : NText::CTextBuffer::getInstance().getMap().keys())
        {
            const auto& list = NText::CTextBuffer::getInstance().getMap()[pattern];
            if (!list.isEmpty())
            {
                setProperty(pattern, list.last().mDataCurrent);
            }
            else
            {
                mEngine->globalObject().deleteProperty(pattern);
            }
        }

        setProperty("HOUR", CDateTime::getCurrentHour());

        setProperty("MINUTE", CDateTime::getCurrentMinute());
        setProperty("SECOND", CDateTime::getCurrentSecond());
        setProperty("DAY_OF_WEEK", CDateTime::getCurrentDayOfWeek());

        setProperty("IS_HOME", CConfig::getInstance().mRunStatus.mHomeStatus
                                            == CRunStatus::HomeStatus::HOME_IN);

        bool hasMotionAlert = false;
        for (size_t i = 0; i < CImageBuffer::getBufferInstance().getBuffersSize(); i++)
        {
            QString s = "BUFF_" + QString::number(i);
            bool value = CImageBuffer::getBufferInstance().getHasAlert(i);
            hasMotionAlert |= value;
            setProperty(s, value);
        }

        setProperty("HAS_MOTION", hasMotionAlert);

        for (const auto& condition : CConfig::getInstance().mConditionList->mConditions)
        {
            LOG_DEBUG << "do expression" << condition.mExpression;
            QJSValue result =  mEngine->evaluate(condition.mExpression).toBool();

            if (result.isError())
            {
                LOG_WARN << "error in expression"  << "\n"
                         << result.property("lineNumber").toInt()
                         << "\n" << ":" << result.toString()
                         << " in expression " << condition.mExpression;
            }
            else
            {
                QString debugStr = "debugStr";
                QString infoStr = "infoStr";
                QString warnStr = "warnStr";
                QString argsStr = "args";
                if (mEngine->globalObject().hasProperty(debugStr))
                {
                    LOG_DEBUG << mEngine->globalObject().property(debugStr).toString();
                }
                if (mEngine->globalObject().hasProperty(infoStr))
                {
                    LOG_INFO << mEngine->globalObject().property(infoStr).toString();
                }
                if (mEngine->globalObject().hasProperty(warnStr))
                {
                    LOG_WARN << mEngine->globalObject().property(warnStr).toString();
                }
                QString argsForCommand;
                if (mEngine->globalObject().hasProperty(argsStr))
                {
                    argsForCommand = mEngine->globalObject().property(argsStr).toString();
                }
                if (result.toBool())
                {
                    doCondition(condition, argsForCommand);
                }
            }


        }
    }

}


}
