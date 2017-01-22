#pragma once

#include <memory>

#include "Image/motionAlgorithms/CAbstractAlertAlgorighm.hpp"
#include "Image/motionAlgorithms/CMoveDetectConfig.hpp"

namespace NAlgorithms
{
class CMoveDetectBitwiseAnd : public CAbstractAlertAlgorighm
{
public:
    CMoveDetectBitwiseAnd(
            const CMoveDetectConfig& config,
            size_t bufferId,
            bool isColor,
            const NVideoInput::CAlgoCoeficients& algoCoef);

    virtual bool hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp> >& buff) override;
    virtual const std::string getAlgorithmName() const override;

    bool hasMoveInGray(const cv::Mat& prev, const cv::Mat& current, const cv::Mat& next);
    virtual ~CMoveDetectBitwiseAnd();

protected:
    CMoveDetectConfig mConfig;
};
}
