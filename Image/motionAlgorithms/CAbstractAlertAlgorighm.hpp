#pragma once

#include <QList>

#include "IAlertAlgorithm.hpp"
#include "Image/CMatWithTimeStamp.hpp"

class CMatWithTimeStamp;

namespace NVideoInput
{
    struct CAlgoCoeficients;
}
namespace NAlgorithms
{
class CAbstractAlertAlgorighm : public IAlertAlgorithm
{
public:
    CAbstractAlertAlgorighm(size_t bufferId, size_t skipFrames);
    virtual const std::string& getInfo() const override;
    virtual void drawContours(cv::Mat& mat) const override;
    virtual size_t getBufferId() const override;
    virtual void reset() override;
    static bool isMatTypeSame(const cv::Mat& mat1, const cv::Mat& mat2);

    virtual bool hasAlertWithSkip(
            const QList<std::shared_ptr<CMatWithTimeStamp> >& buff,
            cv::Mat& mask,
            std::string& info) override;

    virtual ~CAbstractAlertAlgorighm();

    const cv::Mat& getMask() const override;

private:
    static void drawContours(cv::Mat& mat, cv::Mat& mask, const std::vector<std::vector<cv::Point> >& mIntruders); //TODO:

protected:
    cv::Mat mMask;
    std::string mInfo;
    std::vector<std::vector<cv::Point>> mIntruders;
    size_t mBufferId;
    size_t mSkipFrames = 0;
    size_t mCurrentFrame;
};
}
