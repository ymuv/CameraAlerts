#include "CMatWithTimeStampImpl.hpp"
#include "logging/Log.hpp"

namespace
{
    static const int sDefaultCompression = 75;
    const static QString sDateToStrFormat = "yyyy-MM-dd";
    const static QString sDateTimeWithMSToStrFormat = "yyyy-MM-dd_hh-mm-ss.zzz";
}
CMatWithTimeStamp::CMatWithTimeStampImpl::CMatWithTimeStampImpl(
        double scaleFactor, double scaleFactorNet)
    : mScaleFactor(scaleFactor)
    , mNetScaleFactor(scaleFactorNet)
    , mDateTimeCreation(QDateTime::currentDateTime())
    , mDateTimeReceive(QDateTime::currentDateTime())
{
}

CMatWithTimeStamp::CMatWithTimeStampImpl::CMatWithTimeStampImpl(
        const cv::Mat& mat,
        double scaleFactor,
        double scaleFactorNet,
        NTypes::tMatIdType id)
    : CMatWithTimeStampImpl(scaleFactor, scaleFactorNet)
{
    mMatArray[ImageType::IMAGE_RGB_FULL] = mat;
    mId = id;
}

const cv::Mat CMatWithTimeStamp::CMatWithTimeStampImpl::getMask()
{
    writeTime(getDateTimeWithMS(), mId);
    return mMask;
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::setMask(const cv::Mat& mask)
{
    mask.copyTo(mMask);
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::increaseRefCounter()
{
    std::lock_guard<std::mutex> lock(mRefCounterMutex);
    mRefCounter++;
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::decreaseRefCounter()
{
    std::lock_guard<std::mutex> lock(mRefCounterMutex);
    mRefCounter--;
    if (mRefCounter == 0)
    {
        clear();
    }
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::clear()
{
    std::lock_guard<std::recursive_mutex> lock(mMutexGenerateImage);
    if (mBuffer.empty())
    {
        std::vector<int> param(2);
        param[0] = cv::IMWRITE_JPEG_QUALITY;
        param[1] = sDefaultCompression;
        cv::imencode(".jpg", mMatArray[ImageType::IMAGE_RGB_FULL], mBuffer, param);
    }

    for (int i = 0; i < ImageType::MAX_VALUE; i++)
    {
        auto type = static_cast<ImageType::EType>(i);
        mMatArray[type].release();
    }
    mMask.release();
}

bool CMatWithTimeStamp::CMatWithTimeStampImpl::hasMask() const
{
    return !mMask.empty() && mMask.cols > 0 && mMask.rows > 0;
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::writeTime(const std::string& dateTime, size_t id)
{
    if (!mIsDateTimeWrited && !mMask.empty() && mMask.cols > 0 && mMask.rows > 0)
    {
        CMatWithTimeStamp::writeText(mMask, dateTime, std::to_string(id));
    }
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::copyToThis(const cv::Mat& mat) const
{
    mat.copyTo(mMatArray[ImageType::IMAGE_RGB_FULL]);
}

const cv::Mat CMatWithTimeStamp::CMatWithTimeStampImpl::getMat(ImageType::EType type) const
{
    static_assert(ImageType::MAX_VALUE == 6, "Change ImageType enum change");
    std::lock_guard<std::recursive_mutex> lock(mMutexGenerateImage);
    if (type == ImageType::IMAGE_RGB_FULL)
    {
        if (mMatArray[ImageType::IMAGE_RGB_FULL].empty())
        {
            return cv::imdecode(mBuffer, 1);
        }
    }
    else if (type == ImageType::IMAGE_GRAY_FULL)
    {

        processGrayImg();
    }
    else if (type == ImageType::IMAGE_GRAY_RESIZED)
    {
        processGrayImgResized();
    }
    else if (type == ImageType::IMAGE_RGB_RESIZED)
    {
        processColorImgResized();
    }
    else if (type == ImageType::IMAGE_RGB_FULL_WITH_TEXT)
    {
        getMat(ImageType::IMAGE_RGB_FULL).copyTo(
                    mMatArray[ImageType::IMAGE_RGB_FULL_WITH_TEXT]);
        CMatWithTimeStamp::writeText(mMatArray[ImageType::IMAGE_RGB_FULL_WITH_TEXT],
                getDateTimeWithMS(), std::to_string(mId));
    }
    else if (type == ImageType::IMAGE_GRAY_NET_RESIZE)
    {
        if (mMatArray[ImageType::IMAGE_GRAY_NET_RESIZE].empty())
        {
            processGrayImg();
            if (mNetScaleFactor > 1)
            {
                cv::resize(
                        mMatArray[ImageType::IMAGE_GRAY_FULL],
                        mMatArray[ImageType::IMAGE_GRAY_NET_RESIZE],
                        cv::Size(),
                        1./mNetScaleFactor, 1./mNetScaleFactor);
            }
            else
            {
                mMatArray[ImageType::IMAGE_GRAY_NET_RESIZE] =
                        mMatArray[ImageType::IMAGE_GRAY_FULL];
            }
        }
    }

    return mMatArray[type];
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::processGrayImg() const
{
    if (mMatArray[ImageType::IMAGE_GRAY_FULL].empty())
    {
        cv::cvtColor(mMatArray[ImageType::IMAGE_RGB_FULL],
                mMatArray[ImageType::IMAGE_GRAY_FULL],
                CV_BGR2GRAY);
    }
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::processGrayImgResized() const
{
    (void) processGrayImg();
    if (mMatArray[ImageType::IMAGE_GRAY_RESIZED].empty())
    {
        if (mScaleFactor > 1)
        {
            cv::resize(mMatArray[ImageType::IMAGE_GRAY_FULL],
                    mMatArray[ImageType::IMAGE_GRAY_RESIZED],
                    cv::Size(),
                    1./mScaleFactor, 1./mScaleFactor);
        }
        else
        {
            mMatArray[ImageType::IMAGE_GRAY_RESIZED] = mMatArray[ImageType::IMAGE_GRAY_FULL];
        }
    }
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::processColorImgResized() const
{
    if (mMatArray[ImageType::IMAGE_RGB_RESIZED].empty())
    {
        if (mScaleFactor > 1)
        {
            cv::resize(mMatArray[ImageType::IMAGE_RGB_FULL],
                    mMatArray[ImageType::IMAGE_RGB_RESIZED],
                    cv::Size(), 1./mScaleFactor, 1./mScaleFactor);
        }
        else
        {
            mMatArray[ImageType::IMAGE_RGB_RESIZED] = mMatArray[ImageType::IMAGE_RGB_FULL];
        }
    }
}

NTypes::tMatIdType CMatWithTimeStamp::CMatWithTimeStampImpl::getId() const
{
    return mId;
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::setId(const NTypes::tMatIdType& id)
{
    mId = id;
}

const QDateTime&CMatWithTimeStamp::CMatWithTimeStampImpl::getDateTimeCreation() const
{
    return mDateTimeCreation;
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::setDateTimeReceive(const QDateTime& dtReceive)
{
    mDateTimeReceive = dtReceive;
}

const QDateTime&CMatWithTimeStamp::CMatWithTimeStampImpl::getDateTimeReceive() const
{
    return mDateTimeReceive;
}

void CMatWithTimeStamp::CMatWithTimeStampImpl::setDateTimeCreation(const QDateTime& dtCreation)
{
    mDateTimeCreation = dtCreation;
}

const std::string CMatWithTimeStamp::CMatWithTimeStampImpl::getDateTimeWithMS() const
{
    return mDateTimeCreation.toString(sDateTimeWithMSToStrFormat).toStdString();
}

const std::string CMatWithTimeStamp::CMatWithTimeStampImpl::getDate() const
{
    return mDateTimeCreation.toString(sDateToStrFormat).toStdString();
}
