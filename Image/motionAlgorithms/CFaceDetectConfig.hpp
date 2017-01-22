#pragma once

#include <string>

#include "CAlgoBaseConfig.hpp"

struct CFaceDetectConfig : public CAlgoBaseConfig
{
    size_t mMinPositiveFrames;

    bool mIsProcessFirstOnly;
    double mScaleFactorMain;
    double mScaleFactorOther;
    int mMinNeighboursMain;
    int mMinNeighboursOther;
    int mSizeMain;
    int mSizeOther;

    std::string mCascadeHaarPrefix;
    std::vector<std::string> mCascadesHaarMain;
    std::vector<std::string> mCascadesHaarOther;

    std::string mCascadeLBPPrefix;
    std::vector<std::string> mCascadesLBPMain;
    std::vector<std::string> mCascadesLBPOther;
};
