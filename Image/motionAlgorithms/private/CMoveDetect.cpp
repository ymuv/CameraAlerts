#include <opencv2/imgproc/imgproc.hpp>

#include "Image/motionAlgorithms/CMoveDetect.hpp"
#include "Image/videoInput/CInputConfig.hpp"

namespace NAlgorithms
{
namespace
{
template <typename T>
inline bool compare(const T& t1, T& t2)
{
    return std::abs(t1 - t2) < std::numeric_limits<T>::epsilon();
}

template <typename T>
inline bool compare(T t1, T t2)
{
    return std::abs(t1 - t2) < std::numeric_limits<T>::epsilon();
}

template <typename T>
inline bool isNotZero(const T& t1)
{
    return !compare<T>(t1, static_cast<T>(0));
}
} //namespace

using namespace cv;


CMoveDetect::CMoveDetect(
        const CMoveDetectConfig& config,
        size_t bufferId,
        bool isColor,
        const NVideoInput::CAlgoCoeficients& algoCoef)
    : NAlgorithms::CAbstractAlertAlgorighm(bufferId, config.mSkipFrames)
    , mConfig(config)
    , mIsColor(isColor)
{
    //TODO: double compare
    if (algoCoef.mAlgoCoenficient != 1.0)
    {
        mConfig.mAreaMinSize *= algoCoef.mAlgoCoenficient;
    }
    if (algoCoef.mAlgoTrashCoenficient != 1.0)
    {
        mConfig.mMinTrashHold *= algoCoef.mAlgoTrashCoenficient;
    }
}

float CMoveDetect::getNewMoveAreaSize(const CMoveDetectConfig& config, size_t bufferId)
{
    float minAreaSize = config.mAreaMinSize;
//    if (isNotZero(config.mAdoptAreaSizeToFPSCoeficient))
//    {
//       LOG_FATAL << "not implemented";
//        //TODO: config to param
//        auto sleepTime = CConfig::getInstance().mInputConfig->mInputList[bufferId].mSleepTime;
//        //TODO: sleepTime > 0

//        float fps = 1.0 / sleepTime;
//        if (fps > config.mFromWhichFPSAdoptArea)
//        {
//            float k = minAreaSize / fps;
//            double delta = fps - config.mFromWhichFPSAdoptArea;

//            float newSize= k * config.mFromWhichFPSAdoptArea +
//                    (delta * k * config.mAdoptAreaSizeToFPSCoeficient / fps);
//            minAreaSize = newSize;
//        }
//    }
    return minAreaSize;
}

bool CMoveDetect::findContours(
        const CMoveDetectConfig& config,
        cv::Mat& inputMat,
        std::vector<std::vector<cv::Point> >& intruders,
        std::string& info,
        cv::Mat& mask,
        int trashHold)
{
    const cv::Mat& kernelErode = CMoveDetect::getErodeKernel();

    if (config.mIsUseErode)
    {
        erode(inputMat, inputMat, kernelErode);
    }

    if (config.mIsUseDilate)
    {
        dilate(inputMat, inputMat, kernelErode);
    }

    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(inputMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    float minAreaSize = config.mAreaMinSize;

    // Select only large enough contours
    intruders.clear();

    info = "";
    for (size_t i = 0; i < contours.size(); i++)
    {
        cv::Rect rectangle = boundingRect(contours[i]);
        double area = cv::contourArea(contours[i]);

        if (area >= minAreaSize
                && rectangle.height >= config.mMinRectangeHeight
                && rectangle.width >= config.mMinRectangeWidth)
        {
            info += "[area:"  + std::to_string(static_cast<int>(area));
            info += "; min:" + std::to_string(static_cast<int>(minAreaSize));
            if (trashHold)
            {
                info += "; trash:" + std::to_string(trashHold);
            }
            info += "; rect w:" + std::to_string(rectangle.width)
                    + "; h:" + std::to_string(rectangle.height);
            info  +=  "]\n";

            intruders.push_back(contours[i]);
        }
    }

    mask = cv::Mat::zeros(inputMat.size(), CV_8UC3);

    if (intruders.size())
    {
        return true;
    }
    return false;
}

int CMoveDetect::getNewTrashHold(const cv::Mat& a)
{
    int trashHold = mConfig.mMinTrashHold; //TODO: check light

//    if (isNotZero(mConfig.mAdoptTrashHoldToLightCoeficient))
//    {
//        double min, currentMax;
//        cv::minMaxLoc(a, &min, &currentMax);
//        if (currentMax < mConfig.mMintLightFromWhichAdopt)
//        {
//            double K = currentMax / mConfig.mMintLightFromWhichAdopt;
//            trashHold = K * trashHold; //TODO: coeficient

//            const static double minTrashHold = 10;
//            if (trashHold < minTrashHold)
//            {
//                trashHold = minTrashHold;
//            }

//            LOG_DEBUG << "new trashHold " << trashHold;
//        }
//    }
    return trashHold;
}

bool CMoveDetect::hasMove(const cv::Mat& a, const cv::Mat& b)
{
    if (!isMatTypeSame(a,b))
    {
        return false;
    }
    cv::Mat diffMat;
    cv::absdiff(a, b, diffMat);

    std::vector<cv::Mat> channelsVector;
    cv::split(diffMat, channelsVector);
    // Apply threshold to each channel and combine the results
    diffMat = cv::Mat::zeros(diffMat.size(), CV_8UC1);

    size_t size = channelsVector.size();

    int trashHold = getNewTrashHold(a);
    cv::Mat thresh;
    for (size_t i = 0; i < size; i++)
    {
        cv::threshold(channelsVector[i], thresh, trashHold, 255, CV_THRESH_BINARY);
        diffMat |= thresh;
    }

    return findContours(
                mConfig,
                diffMat,
                mIntruders,
                mInfo,
                mMask,
                trashHold);
}

const Mat& CMoveDetect::getErodeKernel()
{
    static const cv::Mat kernelErode = getStructuringElement(MORPH_RECT, Size(2,2));
    return kernelErode;
}

void CMoveDetect::doMorphologyEx(Mat& mat, const CMoveDetectConfig& config)
{
//    if (config.mMorphologyExCloseSize > 0 && config.mMorphologyExCloseIteration > 0)
//    {
//        int size = config.mMorphologyExCloseSize;
//        cv::Mat kernel;
//        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(size,size));
//        cv::morphologyEx(mat, mat, cv::MORPH_CLOSE, kernel, cv::Point(-1,-1), config.mMorphologyExCloseIteration);
//    }

//    if (config.mMorphologyExOpenSize > 0 && config.mMorphologyExOpenIteration > 0)
//    {
//        int size = config.mMorphologyExOpenSize;
//        cv::Mat kernel;
//        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(size,size));
//        cv::morphologyEx(mat, mat, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), config.mMorphologyExOpenIteration);
//    }
}

bool CMoveDetect::hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp> >& buff)
{
    auto size = buff.size();
    assert(size >= 2);
    CMatWithTimeStampLocker lock1(buff[size - 2]);
    CMatWithTimeStampLocker lock2(buff[size - 1]);

    return hasMove(buff[size - 1]->getImgResized(mIsColor),
            buff[size - 2]->getImgResized(mIsColor));
}

const std::string CMoveDetect::getAlgorithmName() const
{
    if (mIsColor)
    {
        return "MOVE_DETECT_COL";
    }
    return "MOVE_DETECT";
}

}
