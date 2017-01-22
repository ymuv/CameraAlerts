#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "Image/motionAlgorithms/CFaceDetect.hpp"
#include "logging/Log.hpp"

namespace NAlgorithms
{
CFaceDetect::CFaceDetect(const CFaceDetectConfig& config, size_t bufferId)
    : CAbstractAlertAlgorighm(bufferId, config.mSkipFrames)
    , mConfig(config)
    , mPositiveFrames(0)
{
    loadCascades();
}

bool CFaceDetect::hasFace(const cv::Mat& frameGray)
{
    mMask = cv::Mat::zeros(frameGray.size(), CV_8UC3);

    bool retVal = false;
    std::vector<cv::Rect> faces;
    std::vector<cv::Rect> eyes;

    for (size_t mainCascadeId = 0; mainCascadeId < mainCascades.size(); mainCascadeId++)
    {
        mainCascades[mainCascadeId].detectMultiScale(
                    frameGray, faces,
                    mConfig.mScaleFactorMain, mConfig.mMinNeighboursMain,
                    0, cv::Size(mConfig.mSizeMain, mConfig.mSizeMain));

        for (size_t i = 0; i < faces.size(); i++)
        {
            cv::rectangle(mMask, faces[i],cv::Scalar(255, 0, 255), 4, 8, 0 );

            cv::Mat faceROI = frameGray(faces[i]);
            //            cv::imshow("maks_" + std::to_string(id++), mask);

            for (size_t cascadeId = 0; cascadeId < otherCascades.size(); cascadeId++)
            {
                eyes.clear();
                otherCascades[cascadeId].detectMultiScale(
                            faceROI, eyes,
                            mConfig.mScaleFactorOther, mConfig.mMinNeighboursOther,
                            0 |CV_HAAR_SCALE_IMAGE, cv::Size(mConfig.mSizeOther, mConfig.mSizeOther));
                for (size_t j = 0; j < eyes.size(); j++)
                {
                    retVal = true;
                    cv::Point center(
                                faces[i].x + eyes[j].x + eyes[j].width * 0.5,
                                faces[i].y + eyes[j].y + eyes[j].height * 0.5);
                    int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
                    cv::circle(mMask, center, radius, cv::Scalar(255, 0, 0), 4, 8, 0);
                    //                   cv::imshow("maks_" + std::to_string(id), mask);

//                    std::cout << faces[i] << std::endl;
                    if (mConfig.mIsProcessFirstOnly)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return retVal;
}

bool CFaceDetect::hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp> >& buff)
{
    return hasFace(buff[buff.size() - 1]->getImgResized(false));
}

const std::string CFaceDetect::getAlgorithmName() const
{
    return "FACE_DETECT";
}

void CFaceDetect::drawContours(cv::Mat& mat) const
{
    if (mat.rows != mMask.rows || mat.cols != mMask.cols)
    {
        cv::Mat t;
        mMask.copyTo(t);
        cv::resize(t, t, cv::Size(mat.cols, mat.rows));

        cv::addWeighted(mat, 1.0, t, 1.0, 0.0, mat);
        return;
    }
    if (mat.type() == mMask.type())
    {
        cv::addWeighted(mat, 1.0, mMask, 1.0, 0.0, mat);
    }
}

void CFaceDetect::loadCascades()
{
    for (const auto& cascadeStr : mConfig.mCascadesHaarMain)
    {
        cv::CascadeClassifier tmp;
        if (!tmp.load(mConfig.mCascadeHaarPrefix+ cascadeStr))
        {
            LOG_WARN << "fail load cascade "
                     << mConfig.mCascadeHaarPrefix + cascadeStr;
        }
        else
        {
            mainCascades.push_back(tmp);
            mainCascadesString.push_back(cascadeStr);
        }
    }

    for (const auto& cascadeStr : mConfig.mCascadesHaarOther)
    {
        cv::CascadeClassifier tmp;
        if (!tmp.load(mConfig.mCascadeHaarPrefix + cascadeStr))
        {
            LOG_WARN << "fail load cascade " << mConfig.mCascadeHaarPrefix + cascadeStr;
        }
        else
        {
            otherCascades.push_back(tmp);
            mainCascadesString.push_back(cascadeStr);
        }
    }

    for (const auto& cascadeStr : mConfig.mCascadesLBPMain)
    {
        cv::CascadeClassifier tmp;
        if (!tmp.load(mConfig.mCascadeLBPPrefix+ cascadeStr))
        {
            LOG_WARN << "fail load cascade "
                     << mConfig.mCascadeLBPPrefix + cascadeStr;
        }
        else
        {
            mainCascades.push_back(tmp);
            mainCascadesString.push_back(cascadeStr);
        }
    }

    for (const auto& cascadeStr : mConfig.mCascadesLBPOther)
    {
        cv::CascadeClassifier tmp;
        if (!tmp.load(mConfig.mCascadeLBPPrefix + cascadeStr))
        {
            LOG_WARN << "fail load cascade " << mConfig.mCascadeLBPPrefix + cascadeStr;
        }
        else
        {
            otherCascades.push_back(tmp);
            mainCascadesString.push_back(cascadeStr);
        }
    }
}

bool CFaceDetect::hasAlertWithSkip(
        const QList<std::shared_ptr<CMatWithTimeStamp> >& buff,
        cv::Mat& mask, std::string& info)
{
    if ((mCurrentFrame++ % mConfig.mSkipFrames) == 0 ||
            mPositiveFrames > 0)
    {
        if (hasAlert(buff))
        {
            mPositiveFrames++;
            if (mPositiveFrames >= mConfig.mMinPositiveFrames)
            {
                drawContours(mask);
                info = getInfo();
                return true;
            }
        }
        else
        {
            mPositiveFrames = 0;
        }
        return false;
    }
    else
    {
        mPositiveFrames = 0;
        return false;
    }
}
}
