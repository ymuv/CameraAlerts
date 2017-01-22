#pragma once

#include <memory>
#include <opencv2/highgui/highgui.hpp>
#include <string>

struct CAlertNotificationConfig;
class CImageBuffer;
class CMatWithTimeStamp;

namespace NAlarms
{
    struct CSMTPAlertConfig;
}


template <class T>
class QList;

namespace cv
{
class Mat;
}

class CVideoWriter
{
public:
    static bool writeVideo(
            const std::string& name,
            const CImageBuffer& buffer,
            size_t bufferID,
            const std::string& info,
            const NAlarms::CSMTPAlertConfig& config,
            bool isWriteMaskAndLastImageToFile = false);
private:
    CVideoWriter() = delete;
public:
    static const int sVideoType;
};
