#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Image/motionAlgorithms/CPHash.hpp"
#include "Image/motionAlgorithms/CPHashConfig.hpp"
#include "logging/Log.hpp"

namespace NAlgorithms
{

CPHash::CPHash(const CPHashConfig& conf, size_t bufferId)
    : CAbstractAlertAlgorighm(bufferId, conf.mSkipFrames)
    , mConfig(conf)
{
}

const std::string CPHash::getAlgorithmName() const
{
    return "PHash";
}

bool CPHash::hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp> >& buff)
{
    auto size = buff.size();
    assert(size >= 2);
    CMatWithTimeStampLocker lock1(buff[size - 2]);
    CMatWithTimeStampLocker lock2(buff[size - 1]);

    const auto hash1 = pHashValue(buff[size - 1]->getImgResized(false));
    const auto hash2 = pHashValue(buff[size - 2]->getImgResized(false));
    auto distance = calcHammingDistance(hash1, hash2);
    mInfo = "hemming dist: " + std::to_string(distance) + " min: " + std::to_string(mConfig.mMinDistance);
    return distance > mConfig.mMinDistance;
}

int64 CPHash::pHashValue(const cv::Mat& src)
{
    cv::Mat img, dst;
    cv::resize(src, img, cv::Size(32, 32));
    img = cv::Mat_<double>(img);

    cv::dct(img, dst);

    dst = dst(cv::Rect(1, 1, 8, 8) );

    cv::Scalar imageMean = cv::mean(dst);

    int64 hash = 0;
    int counter = 0;
    mMask = (dst >= imageMean[0]);

    for (int i = 0; i < mMask.rows; i++)
    {
        for (int j = 0; j < mMask.cols; j++)
        {
            if (mMask.at<uchar>(i,j))
            {
                hash |= (int64) 1<< counter;
            }
            counter++;
        }
    }
    return hash;
}

int64 CPHash::calcHammingDistance(int64 x, int64 y) const
{
    int64 dist = 0, val = x ^ y;
    while(val)
    {
        ++dist;
        val &= val - 1;
    }
    return dist;
}

}
