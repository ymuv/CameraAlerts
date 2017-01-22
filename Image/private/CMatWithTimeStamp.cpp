#include <opencv2/highgui/highgui.hpp>

#include "Image/CMatWithTimeStamp.hpp"
#include "CMatWithTimeStampImpl.hpp"


CMatWithTimeStamp::CMatWithTimeStamp(
        const cv::Mat& img,
        double scaleFactor,
        double scaleFactorNet,
        NTypes::tMatIdType id)
    : mCMatWithTimeStampImpl(
          new CMatWithTimeStampImpl(img, scaleFactor, scaleFactorNet, id))
{
}

CMatWithTimeStamp::CMatWithTimeStamp(double scaleFactor)
    : mCMatWithTimeStampImpl(new CMatWithTimeStampImpl(scaleFactor))
{
}

CMatWithTimeStamp::~CMatWithTimeStamp()
{
    mDtorWork = true;
}

void CMatWithTimeStamp::writeText(
        cv::Mat& mat,
        const std::string& str,
        const std::string& strSmall,
        double fontScale)
{
    if (mat.empty())
    {
        return;
    }
    int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
    double fontScaleSmall = fontScale * 2.0 / 3.0;
    int thickness = 1;
    int baseline=0;

    cv::Size textSize = cv::getTextSize(str, fontFace, fontScale, thickness, &baseline);
    cv::Size textSizeSmall = cv::getTextSize(strSmall, fontFace, fontScaleSmall, thickness, &baseline);

    static const int textOffset = 5;
    cv::Point textPos((mat.cols - (textSize.width  + textOffset) - (textSizeSmall.width + textOffset)), //X
                      (mat.rows - textSize.height + textOffset)); //y

    if (!strSmall.empty())
    {
        cv::Point textPosForSmallText((mat.cols - (textSizeSmall.width + textOffset)), //X
                                      (mat.rows - textSize.height + textOffset)); //y
        cv::putText(mat, strSmall, textPosForSmallText,
                    fontFace, fontScaleSmall, cv::Scalar(0, 0, 255), thickness, 22);
    }

    if (str.find("\n") != std::string::npos)
    {
        static int offset = 5;
        int lineId = 0;
        std::stringstream ss(str);
        std::string line;

        while(std::getline(ss, line,'\n'))
        {
            cv::putText(mat,
                        line,
                        cv::Point(20, 20 + lineId* (textSize.height + offset)),
                        fontFace,
                        fontScale,
                        cv::Scalar(255, 255, 255),
                        thickness,
                        8,
                        false);
            lineId ++;
        }
    }
    else
    {
        cv::putText(mat, str, textPos,
                    fontFace, fontScale, cv::Scalar(0, 0, 255), thickness, 22);
    }
}

const cv::Mat CMatWithTimeStamp::getMat(ImageType::EType type) const
{
    return mCMatWithTimeStampImpl->getMat(type);
}

void CMatWithTimeStamp::copyToThis(const cv::Mat& mat) const
{
    mCMatWithTimeStampImpl->copyToThis(mat);
}

const cv::Mat CMatWithTimeStamp::getImgResized(bool isColor) const
{
    if (isColor)
    {
        return getMat(ImageType::IMAGE_RGB_RESIZED);
    }
    return getMat(ImageType::IMAGE_GRAY_RESIZED);
}

void CMatWithTimeStamp::increaseRefCounter()
{
    mCMatWithTimeStampImpl->increaseRefCounter();
}

void CMatWithTimeStamp::decreaseRefCounter()
{
    mCMatWithTimeStampImpl->decreaseRefCounter();
}

const cv::Mat CMatWithTimeStamp::getMask() const
{
    return mCMatWithTimeStampImpl->getMask();
}

void CMatWithTimeStamp::setMask(const cv::Mat& mask)
{
    mCMatWithTimeStampImpl->setMask(mask);
}

bool CMatWithTimeStamp::hasMask() const
{
    return mCMatWithTimeStampImpl->hasMask();
}

const QDateTime&CMatWithTimeStamp::getDateTimeCreation() const
{
    return mCMatWithTimeStampImpl->getDateTimeCreation();
}

void CMatWithTimeStamp::setDateTimeCreation(const QDateTime& dtCreation)
{
    mCMatWithTimeStampImpl->setDateTimeCreation(dtCreation);
}

void CMatWithTimeStamp::setDateTimeReceive(const QDateTime& dtReceive)
{
    mCMatWithTimeStampImpl->setDateTimeReceive(dtReceive);
}
const QDateTime&CMatWithTimeStamp::getDateTimeReceive() const
{
    return mCMatWithTimeStampImpl->getDateTimeReceive();
}

std::string CMatWithTimeStamp::saveTo(const std::string& filePath, size_t buffId)
{
    std::string fileName = filePath
            + std::to_string(buffId) + "_"
            + mCMatWithTimeStampImpl->getDateTimeWithMS() + "__"
            + std::to_string(mCMatWithTimeStampImpl->getId()) + ".jpg";

    return cv::imwrite(fileName, getMat()) ? fileName : "";
}

const std::string CMatWithTimeStamp::getDateTimeWithMS() const
{
    return mCMatWithTimeStampImpl->getDateTimeWithMS();
}

const std::string CMatWithTimeStamp::getDate() const
{
    return mCMatWithTimeStampImpl->getDate();
}

NTypes::tMatIdType CMatWithTimeStamp::getId() const
{
    return mCMatWithTimeStampImpl->getId();
}

void CMatWithTimeStamp::setID(NTypes::tMatIdType id)
{
    return mCMatWithTimeStampImpl->setId(id);
}

CMatWithTimeStampLocker::CMatWithTimeStampLocker(std::shared_ptr<CMatWithTimeStamp> mat)
    : mMat(mat)
{
    mMat->increaseRefCounter();
}

CMatWithTimeStampLocker::~CMatWithTimeStampLocker()
{
    mMat->decreaseRefCounter();
}
