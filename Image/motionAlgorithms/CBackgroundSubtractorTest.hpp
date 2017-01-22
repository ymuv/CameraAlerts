#pragma once
#include <opencv2/opencv.hpp>

#include "CAbstractAlertAlgorighm.hpp"
#include "CMoveDetectBackgroundSubtractorConfig.hpp"
namespace NAlgorithms
{
class CMoveDetectBackgroundSubtractorTest: public CAbstractAlertAlgorighm
{
public:
    CMoveDetectBackgroundSubtractorTest(
            const CMoveDetectBackgroundSubtractorConfig& config,
            size_t bufferId,
            bool isColor,
            const NVideoInput::CAlgoCoeficients& algoCoef);

    virtual bool hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp> >& buff) override;

    virtual const std::string getAlgorithmName() const override;

    bool test(
            const cv::Mat& mat1,
            const cv::Mat& mat2);

    virtual void reset() override;

private:
    CMoveDetectBackgroundSubtractorConfig mConfig;

    bool mIsColor;
    cv::Mat mAvgMat;
};


}
