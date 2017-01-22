#include "opencv2/opencv.hpp"

#include "Image/motionAlgorithms/CMoveDetectBitwiseAnd.hpp"
#include "Image/motionAlgorithms/CMoveDetect.hpp"
#include "Image/videoInput/CInputConfig.hpp"

namespace NAlgorithms
{

CMoveDetectBitwiseAnd::CMoveDetectBitwiseAnd(
        const CMoveDetectConfig& config,
        size_t bufferId,
        bool isColor,
        const NVideoInput::CAlgoCoeficients& algoCoef)
    : NAlgorithms::CAbstractAlertAlgorighm(bufferId, config.mSkipFrames)
    , mConfig(config)
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

bool CMoveDetectBitwiseAnd::hasAlert(
        const QList<std::shared_ptr<CMatWithTimeStamp> >& buff)
{
    auto size = buff.size();
    assert(size >= 3);
    CMatWithTimeStampLocker lock1(buff[size - 3]);
    CMatWithTimeStampLocker lock2(buff[size - 2]);
    CMatWithTimeStampLocker lock3(buff[size - 1]);
    return hasMoveInGray(
                buff[size - 1]->getImgResized(false),
                buff[size - 2]->getImgResized(false),
                buff[size - 3]->getImgResized(false));
}

const std::string CMoveDetectBitwiseAnd::getAlgorithmName() const
{
    return "MOVE_DETECT_BitwiseAnd";
}

bool CMoveDetectBitwiseAnd::hasMoveInGray(const cv::Mat& prev, const cv::Mat& current, const cv::Mat& next)
{
    using namespace cv;
    Mat diff;
    if (!isMatTypeSame(prev, current) || !isMatTypeSame(current, next))
    {
        return false;
    }

    absdiff(current, next, diff);
    {
        Mat diffPrevCurrent;
        absdiff(prev, current, diffPrevCurrent);
        bitwise_and(diffPrevCurrent,
                    diff,
                    diff);
    }

    threshold(diff,
              diff,
              mConfig.mMinTrashHold,
              255,
              CV_THRESH_BINARY);

    bool result = NAlgorithms::CMoveDetect::findContours(
                mConfig,
                diff,
                mIntruders,
                mInfo,
                mMask,
                mConfig.mMinTrashHold);

//    cv::imshow(getAlgorithmName() + std::to_string(mBufferId) + "_prev", prev);
//    cv::imshow(getAlgorithmName() + std::to_string(mBufferId),
//               mMaskList.at((int) MaskListNumbers::MOTION_TRASH)->mMask);
//    cv::imshow(getAlgorithmName() + std::to_string(mBufferId) + "mask",
//               mMaskList.at((int) MaskListNumbers::MASK)->mMask);
//    cv::waitKey(1);

    return result;

}

CMoveDetectBitwiseAnd::~CMoveDetectBitwiseAnd()
{
}

} //namespace NAlgorithms
