#pragma once
#include <string>

#include "CAlgoBaseConfig.hpp"

struct CMoveDetectConfig : CAlgoBaseConfig
{
    size_t mAreaMinSize;
    int mMinTrashHold;

    bool mIsUseErode;
    bool mIsUseDilate;

    int mMinRectangeWidth;
    int mMinRectangeHeight;

    int mGaussianBlurIndexBegin;
    int mGaussianBlurIndexEnd;
};

std::ostream&  operator<<(std::ostream& stream, const CMoveDetectConfig&);
