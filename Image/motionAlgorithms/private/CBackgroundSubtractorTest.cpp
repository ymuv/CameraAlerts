#include "Image/motionAlgorithms/CBackgroundSubtractorTest.hpp"
#include "Image/motionAlgorithms/CMoveDetect.hpp"
#include "Image/videoInput/CInputConfig.hpp"
#include "logging/Log.hpp"

namespace NAlgorithms
{
CMoveDetectBackgroundSubtractorTest::CMoveDetectBackgroundSubtractorTest(
        const CMoveDetectBackgroundSubtractorConfig& config,
        size_t bufferId,
        bool isColor,
        const NVideoInput::CAlgoCoeficients& algoCoef)
    : CAbstractAlertAlgorighm(bufferId, config.mSkipFrames)
    , mConfig(config)
    , mIsColor(isColor)
{
    if (algoCoef.mAlgoCoenficient != 1.0)
    {
        mConfig.mAreaMinSize *= algoCoef.mAlgoCoenficient;
    }
    if (algoCoef.mAlgoTrashCoenficient != 1.0)
    {
        mConfig.mMinTrashHold *= algoCoef.mAlgoTrashCoenficient;
    }
}

bool CMoveDetectBackgroundSubtractorTest::test(
        const cv::Mat& mat1,
        const cv::Mat& mat2)
{
    cv::Mat diffMat;
    std::vector<cv::Mat> channelsVector;
    {
        cv::Mat absDiffMat;
        cv::absdiff(mat1, mat2, absDiffMat);
        cv::split(absDiffMat, channelsVector);
        diffMat = cv::Mat::zeros(absDiffMat.size(), CV_8UC1);
    }

    size_t channelSize = channelsVector.size();

    int trashHold = mConfig.mMinTrashHold;

    cv::Mat thresh;
    for (size_t i = 0; i < channelSize; i++)
    {
        cv::threshold(channelsVector[i], thresh, trashHold, 255, CV_THRESH_BINARY);
        diffMat |= thresh;
    }

  bool result = CMoveDetect::findContours(
                mConfig,
                diffMat,
                mIntruders,
                mInfo,
                mMask,
                0);

    return result;
}

void CMoveDetectBackgroundSubtractorTest::reset()
{
    mAvgMat.release();

}

bool NAlgorithms::CMoveDetectBackgroundSubtractorTest::hasAlert(
        const QList<std::shared_ptr<CMatWithTimeStamp> >& buff)
{
    auto size = buff.size();

    if (mConfig.mHistoryLength >= size)
    {
        LOG_WARN << "history length > buff size, return false" << size << "hist: " << mConfig.mHistoryLength;
        return false;
    }

    if (!isMatTypeSame(buff[size - 1]->getImgResized(mIsColor), mAvgMat))
    {
        return false;
    }
    CMatWithTimeStampLocker lock(buff[size - 1]);

    if (mAvgMat.empty() || mConfig.mSkipFrames > 1)
    {
        mAvgMat = buff[size - 1]->getImgResized(mIsColor) / 2;

        for (int i =1; i <= mConfig.mHistoryLength; i++)
        {
            mAvgMat += (1./(i*4)) * buff[size - 1 - i]->getImgResized(mIsColor);
        }
    }
    else
    {
        mAvgMat = mAvgMat / 2 + buff[size - 1]->getImgResized(mIsColor) / 2;
    }

    bool result = test(mAvgMat, buff[size - 1]->getImgResized(mIsColor));

    return result;
}

const std::string CMoveDetectBackgroundSubtractorTest::getAlgorithmName() const
{
    if (mIsColor)
    {
        return "MD_BS_MY_COL";
    }
    return "MD_MY_BS";
}

}
