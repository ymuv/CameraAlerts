#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Image/motionAlgorithms/CAbstractAlertAlgorighm.hpp"
#include "common/CMainFunctions.hpp"
#include "logging/Log.hpp"

namespace
{
static int gDivideMatCoef = 1;

}
namespace NAlgorithms
{

CAbstractAlertAlgorighm::CAbstractAlertAlgorighm(
      size_t bufferId,
      size_t skipFrames)
    : mBufferId(bufferId)
    , mSkipFrames(skipFrames)
    , mCurrentFrame(0)
{
}

const std::string& CAbstractAlertAlgorighm::getInfo() const
{
    return mInfo;
}

void CAbstractAlertAlgorighm::drawContours(cv::Mat& mat) const
{
    const static cv::Scalar sColorRedRect = cv::Scalar(255, 0, 255);
    const static cv::Scalar sColor2Rect = cv::Scalar(0, 255, 0);
    const static int sRectRedThickness = 3;
    const static int sRect2Thickness = 1;
    const static int sRectRedLineType = 8;

    cv::drawContours(mMask, mIntruders, -1, CV_RGB(255,255, 255), -1);

    if (mIntruders.size())
    {
        cv::Mat t;
        mMask.copyTo(t);

        if (mat.rows != mMask.rows || mat.cols != mMask.cols)
        {
            cv::resize(t, t, cv::Size(mat.cols, mat.rows));
            mat = (mat / gDivideMatCoef & ~t) + (mat & t);
            return;
        }

        mat = (mat / gDivideMatCoef & ~mMask) + (mat & mMask);
        cv::drawContours(mat, mIntruders, -1, CV_RGB(255,255, 0), 2);
        cv::drawContours(mat, mIntruders, -1, CV_RGB(255,0, 0), 1);
        for (size_t i = 0; i < mIntruders.size(); i++)
        {
            cv::Rect rect = cv::boundingRect(mIntruders[i]);
            cv::rectangle(mat, rect, sColorRedRect, sRectRedThickness, sRectRedLineType);
            cv::rectangle(mat, rect, sColor2Rect, sRect2Thickness, sRectRedLineType);
        }
    }
}

CAbstractAlertAlgorighm::~CAbstractAlertAlgorighm()
{
}

const cv::Mat&CAbstractAlertAlgorighm::getMask() const
{
    return mMask;
}

size_t CAbstractAlertAlgorighm::getBufferId() const
{
    return mBufferId;
}

void CAbstractAlertAlgorighm::reset()
{
    //empty
}

bool CAbstractAlertAlgorighm::isMatTypeSame(const cv::Mat& mat1, const cv::Mat& mat2)
{
    return CMainFunctions::isMatTypeSame(mat1, mat2);
}

bool CAbstractAlertAlgorighm::hasAlertWithSkip(
        const QList<std::shared_ptr<CMatWithTimeStamp> >& buff,
        cv::Mat& mask,
        std::string& info)
{
    try
    {
        if ((mSkipFrames == 0) || (mCurrentFrame++ % mSkipFrames) == 0)
        {
            bool result = hasAlert(buff);
            if (result)
            {
                drawContours(mask);
                info = getInfo();
                return result;
            }
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERROR << "catch exception" << exc.what() << "when try process alog"  << getAlgorithmName();
    }
    return false;
}

void CAbstractAlertAlgorighm::drawContours(cv::Mat& mat, cv::Mat& mask, const std::vector<std::vector<cv::Point>>& intruders)
{
    const static cv::Scalar sColorRect = cv::Scalar(255, 0, 255);
    const static int sRectThickness = 1;
    const static int sRectLineType = 8;

    cv::drawContours(mask, intruders, -1, CV_RGB(255,255, 255), -1);

    if (intruders.size())
    {
        cv::Mat t;
        mask.copyTo(t);

        if (mat.rows != mask.rows || mat.cols != mask.cols)
        {
            cv::resize(t, t, cv::Size(mat.cols, mat.rows));
            mat = (mat / gDivideMatCoef & ~t) + (mat & t);
            return;
        }

        mat = (mat / gDivideMatCoef & ~mask) + (mat & mask);
        cv::drawContours(mat, intruders, -1, CV_RGB(255,255, 0), 2);
        for (size_t i = 0; i < intruders.size(); i++)
        {
            cv::Rect rect = cv::boundingRect(intruders[i]);
            cv::rectangle(mat, rect, sColorRect, sRectThickness, sRectLineType);
        }
    }
}

IAlertAlgorithm::~IAlertAlgorithm()
{
}

}
