#pragma once

namespace NAlarms
{

struct CAlertConfig
{
    int mForceSendInterval;
    int mMinIntervalRepeatSend;
    int mReplyDiffTime;  ///if has alert, after this second send again
    int mFirstTimeSkip;  ///skip N first second (don't send alert messages)
    int mReplyCount;
};
}
