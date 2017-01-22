#pragma once
#include <QDateTime>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <mutex>

#include "Image/CMatWithTimeStamp.hpp"

class CMatWithTimeStamp::CMatWithTimeStampImpl
{
public:
    CMatWithTimeStampImpl(double scaleFactor, double scaleFactorNet = 2.0);

    CMatWithTimeStampImpl(
            const cv::Mat& mat,
            double scaleFactor,
            double scaleFactorNet,
            NTypes::tMatIdType id);

    const cv::Mat getMask();
    void setMask(const cv::Mat& mask);

    void increaseRefCounter();
    void decreaseRefCounter();

    bool hasMask() const;

    void writeTime(const std::string& dateTime, size_t id);

    void copyToThis(const cv::Mat&mat) const;
    const cv::Mat getMat(ImageType::EType type) const;

    const std::string getDateTimeWithMS() const;

    const std::string getDate() const;

    NTypes::tMatIdType getId() const;
    void setId(const NTypes::tMatIdType& id);

    const QDateTime& getDateTimeCreation() const;
    void setDateTimeReceive(const QDateTime& dtReceive);
    const QDateTime& getDateTimeReceive() const;
    void setDateTimeCreation(const QDateTime& dtCreation);

private:
    void processGrayImg() const;
    void processGrayImgResized() const;
    void processColorImgResized() const;

private:
    /**
     * @brief clear remove all mat, save rgb full to buffer
     */
    void clear();

private:
    cv::Mat mMask;
    mutable cv::Mat mMatArray[ImageType::MAX_VALUE];
    bool mIsDateTimeWrited = false;
    double mScaleFactor;
    double mNetScaleFactor;
    NTypes::tMatIdType mId;
    int mRefCounter = 1;
    std::vector<uchar> mBuffer;

    mutable std::recursive_mutex mMutexGenerateImage;
    std::mutex mRefCounterMutex;

    QDateTime mDateTimeCreation;
    QDateTime mDateTimeReceive;
};

