#pragma once

#include "CAbstractAlertAlgorighm.hpp"
#include "CMoveDetectConfig.hpp"

namespace NAlgorithms
{
class CMoveDetect : public NAlgorithms::CAbstractAlertAlgorighm
{
public: //methods
    CMoveDetect(
            const CMoveDetectConfig& config,
            size_t bufferId,
            bool isColor,
            const NVideoInput::CAlgoCoeficients& algoCoef);

    bool hasMove(const cv::Mat& a, const cv::Mat& b);

    virtual bool hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp> >& buff) override;

    const std::string getAlgorithmName() const override;

public: //static
    static float getNewMoveAreaSize(const CMoveDetectConfig& config, size_t bufferId);

    static const cv::Mat& getErodeKernel();

    //remove if not use
    static void doMorphologyEx(cv::Mat& mat, const CMoveDetectConfig& config);

    static bool findContours(
            const CMoveDetectConfig& config,
            cv::Mat& inputMat,
            std::vector<std::vector<cv::Point> >& intruders,
            std::string& info,
            cv::Mat& mask,
            int trashHold);

private: //static
    int getNewTrashHold(const cv::Mat& a);
    bool testSobel(const cv::Mat& a, const cv::Mat& b);

private: //fields
    CMoveDetectConfig mConfig;
    bool mIsColor;
};
}
