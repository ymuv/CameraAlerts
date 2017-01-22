#pragma once

#include "CMoveDetectConfig.hpp"

struct CMoveDetectBackgroundSubtractorConfig : public CMoveDetectConfig
{
    int mHistoryLength;
};
