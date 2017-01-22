#pragma once
namespace NAlarms
{

struct CAlertCoefConfig
{
    float mAlertMinIntervalRepeatSendCoef = 1; //mMinIntervalRepeatSend * mAlertSendCoef
    int mAlertReplyCountCoef = 1; //mReplyCount * mReplyCountCoef
};
}
