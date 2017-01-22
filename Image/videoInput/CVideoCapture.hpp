#pragma once

#include <opencv2/highgui/highgui.hpp>

#include "Image/CMatWithTimeStamp.hpp"

class CImageBuffer;

namespace NVideoInput
{
struct CInputConfig;

class CVideoCapture
{
public: //methods
    explicit CVideoCapture(const CInputConfig& inputConfig);

    bool open(const size_t buffeId);

    /**
     * @brief processNextFrame fill next frame
     * @param bufferId
     */
    bool processNextFrame(const size_t bufferId);

    size_t getBufferSizeWithoutConnection() const;
    ~CVideoCapture();

private: //fields
    const CInputConfig& mInputConfig;
    CImageBuffer& mImageBuffer;
    std::vector<cv::VideoCapture> mInputCaptures;
    size_t mBufferSizeWithoutConnection;
};
}
