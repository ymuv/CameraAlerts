#pragma once

#include <memory>
#include <opencv2/core/core.hpp>

class CMatWithTimeStamp;
class CImageBuffer;

template <typename T>
class QList;

namespace NAlgorithms
{
class IAlertAlgorithm
{
public:
    virtual const cv::Mat& getMask() const = 0;

    virtual const std::string getAlgorithmName() const = 0;

    virtual void reset() = 0;
    virtual size_t getBufferId() const = 0;

    virtual bool hasAlertWithSkip(
            const QList<std::shared_ptr<CMatWithTimeStamp> >& buff,
            cv::Mat& mask,
            std::string& info)  = 0;

    virtual ~IAlertAlgorithm();

public: //static
    static const int sMinAlgoFrameCount = 3;

protected:
    /**
     * @brief getInfo info why alert happend
     * @return info why alert happend
     */
    virtual const std::string& getInfo() const = 0;
    virtual void drawContours(cv::Mat& mat) const = 0;
    virtual bool hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp>>& buff) = 0;
};
}
using pAlertAlgorithm = std::shared_ptr<NAlgorithms::IAlertAlgorithm>;
using algorighmListPtr = std::vector<pAlertAlgorithm>;
