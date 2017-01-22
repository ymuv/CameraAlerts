#pragma once
#include <memory>
#include <vector>

struct CMoveDetectConfig;
struct CMoveDetectBackgroundSubtractorConfig;
struct CFaceDetectConfig;
namespace NAlgorithms
{
    struct CPHashConfig;
}
struct CAlgoBaseConfig;

struct CAlgoConfig
{
    std::shared_ptr<CMoveDetectConfig> mMoveDetectConfig;
    std::shared_ptr<CMoveDetectConfig> mMoveDetectConfigColor;
    std::shared_ptr<CMoveDetectConfig> mMoveDetectBitwiseAnd;

    std::shared_ptr<CMoveDetectBackgroundSubtractorConfig> mMoveDetectBackgroundSubtractorTestConfig;
    std::shared_ptr<CMoveDetectBackgroundSubtractorConfig> mMoveDetectBackgroundSubtractorTestColorConfig;

    std::shared_ptr<CFaceDetectConfig> mFaceDetectConfig;
    std::shared_ptr<NAlgorithms::CPHashConfig> mPHashConfig;
public: //static methods
    static CAlgoConfig& getInstance();

private:
    CAlgoConfig();

    CAlgoConfig(CAlgoConfig&) = delete;
    CAlgoConfig(CAlgoConfig&&) = delete;
    CAlgoConfig& operator=(CAlgoConfig const&) = delete;
    CAlgoConfig& operator=(CAlgoConfig const&&) = delete;
};
std::ostream& operator<<(std::ostream& stream, const CAlgoConfig&);

