#include <ostream>

#include "Image/motionAlgorithms/CPHashConfig.hpp"
#include "Image/motionAlgorithms/CFaceDetectConfig.hpp"
#include "Image/motionAlgorithms/CMoveDetectConfig.hpp"
#include "Image/motionAlgorithms/CMoveDetectBackgroundSubtractorConfig.hpp"
#include "common/CAlgoConfig.hpp"

CAlgoConfig::CAlgoConfig()
{
    mMoveDetectConfig.reset(new CMoveDetectConfig);
    mMoveDetectConfigColor.reset(new CMoveDetectConfig);
    mMoveDetectBitwiseAnd.reset(new CMoveDetectConfig);
    mMoveDetectBackgroundSubtractorTestConfig.reset(new CMoveDetectBackgroundSubtractorConfig);
    mMoveDetectBackgroundSubtractorTestColorConfig.reset(new CMoveDetectBackgroundSubtractorConfig);
    mFaceDetectConfig.reset(new CFaceDetectConfig);
    mPHashConfig.reset(new NAlgorithms::CPHashConfig);
}

CAlgoConfig& CAlgoConfig::getInstance()
{
    static CAlgoConfig sAlgoConfigInstance;
    return sAlgoConfigInstance;
}

std::ostream&operator<<(std::ostream& stream, const CAlgoConfig& config)
{
#ifdef __BUILD_MOTION_ALGO__
    stream << "AlgoConfig:[\n";

//    stream << ";\n PHash:" << *config.mPHashConfig;
    stream << "\n MoveDetectConfig:" << *config.mMoveDetectConfig;
    stream << ";\n MoveDetectConfigColor:" << *config.mMoveDetectConfigColor;
    stream << ";\n MoveDetectBitwiseAnd:" << *config.mMoveDetectBitwiseAnd;
    stream << ";\n MoveDetectBackgroundSubtractorTestConfig:" << *config.mMoveDetectBackgroundSubtractorTestConfig;
    stream << ";\n MoveDetectBackgroundSubtractorTestColorConfig:" << *config.mMoveDetectBackgroundSubtractorTestColorConfig;
    stream << "]";
#endif //__BUILD_MOTION_ALGO__
    return stream;
}
