#pragma once

#include <memory>

#include "Image/ImageType.hpp"
#include "common/NTypes.hpp"

namespace cv
{
class Mat;
}
class CMatWithTimeStamp;

class CMatWithTimeStampLocker
{
public:
    explicit CMatWithTimeStampLocker(std::shared_ptr<CMatWithTimeStamp> mat);
    ~CMatWithTimeStampLocker();
private:
    std::shared_ptr<CMatWithTimeStamp> mMat;
};

class QDateTime;

class CMatWithTimeStamp
{
public: //methods
    CMatWithTimeStamp(
            const cv::Mat& img,
            double scaleFactor,
            double scaleFactorNet,
            NTypes::tMatIdType id);
    explicit CMatWithTimeStamp(double scaleFactor = 0);
    ~CMatWithTimeStamp();

    std::string saveTo(const std::string& filePath, size_t buffId);

    const std::string getDateTimeWithMS() const;
    const std::string getDate() const;

    NTypes::tMatIdType getId() const;
    void setID(NTypes::tMatIdType id);

    const cv::Mat getMat(ImageType::EType type = ImageType::IMAGE_RGB_FULL) const;
    void copyToThis(const cv::Mat& mat) const;

    const cv::Mat getImgResized(bool isColor) const;
    /**
     * remove all mat, save rgb full to buffer
     * */
    void increaseRefCounter();
    void decreaseRefCounter();

    void setMask(const cv::Mat& mask);
    bool hasMask() const;
    const cv::Mat getMask() const;

    const QDateTime& getDateTimeCreation() const;
    void setDateTimeCreation(const QDateTime& dtCreation);

    const QDateTime& getDateTimeReceive() const;
    void setDateTimeReceive(const QDateTime& dtReceive);

public: //static
    static void writeText(
            cv::Mat& mat,
            const std::string& str,
            const std::string& strSmall = "",
            double fontScale = 0.50);

private: //fields
    class CMatWithTimeStampImpl;
    std::shared_ptr<CMatWithTimeStampImpl> mCMatWithTimeStampImpl;

    bool mDtorWork = false; //TODO: test, for debug only
};

using pMatWithTimeStamp=std::shared_ptr<CMatWithTimeStamp>;
