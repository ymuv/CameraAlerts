#pragma once

#include <opencv2/objdetect/objdetect.hpp>
#include <vector>

#include "CAbstractAlertAlgorighm.hpp"
#include "CFaceDetectConfig.hpp"
namespace NAlgorithms
{
class CFaceDetect: public CAbstractAlertAlgorighm
{
public: //methods
    CFaceDetect(const CFaceDetectConfig& config, size_t bufferId);

    /**
     * @brief hasFace
     * @param mat
     * @return true if found some of mainCascades in image and other cascade in mainCascade
     */
    bool hasFace(const cv::Mat& mat);

public: // IAlertAlgorithm interface
    virtual bool hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp> >& buff) override;

    virtual const std::string getAlgorithmName() const override;

    virtual void drawContours(cv::Mat& mat) const override;

private: //methods
    void loadCascades();

private:
    const CFaceDetectConfig& mConfig;
    std::vector<cv::CascadeClassifier> otherCascades;
    std::vector<cv::CascadeClassifier> mainCascades;
    std::vector<std::string> mainCascadesString;
    size_t mPositiveFrames;

    virtual bool hasAlertWithSkip(
            const QList<std::shared_ptr<CMatWithTimeStamp> >& buff,
            cv::Mat& mask,
            std::string& info) override;
};
}
