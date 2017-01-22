#pragma once

#include <string>
#include <vector>

#include "Net/CNetConnectionConfig.hpp"

namespace NVideoInput
{

enum class InputType
{
    CAMERA,
    URL,
    FILE,
};

 // split??

struct CAlgoCoeficients
{
    double mAlgoCoenficient;
    double mAlgoTrashCoenficient;
    bool mIsRunMotionAlgo = true;
};

struct CInputConfigEntry
{
    double mResizeScaleFactor;
    double mResizeScaleFactorForNet;

    double mSleepTime;
    size_t mBufferSize;
    int mLongBufferSize;
    int mLongBufferSkip;

    InputType mType;
    std::string mFile;

    int mCameraId;
    int mFrameWidth;
    int mFrameHeight;

    int mTimeOut; //o double
    NNet::CNetConnectionConfig mConnectionConfig;
    CAlgoCoeficients mAlgoCoeficient;
};

struct CInputConfig
{
    std::vector<CInputConfigEntry> mInputList;
};
}

QDebug operator<<(QDebug stream, const NVideoInput::CInputConfig& config);
