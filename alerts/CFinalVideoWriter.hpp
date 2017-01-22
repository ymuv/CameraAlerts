#pragma once

#include <mutex>
#include <vector>
#include <string>

#include "alerts/IAlert.hpp"
#include "common/NTypes.hpp"

namespace NAlarms
{
struct CFinalVideoWriterConfig;

class CFinalVideoWriter : public IAlert
{
public:
    static std::shared_ptr<CFinalVideoWriter> getInstance(const CFinalVideoWriterConfig& config);
public:
    virtual void onAlert(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo) override;

    virtual void checkAndSendForce(const CImageBuffer& buff, const std::vector<size_t>& bufferIds) override;
    virtual void checkAndSendReply(const CImageBuffer& buff, const std::vector<size_t>& bufferIds) override;
private:
    explicit CFinalVideoWriter(const CFinalVideoWriterConfig& config);

    bool writeFinalVideo(
                const CImageBuffer& buff,
                size_t bufferId,
                cv::VideoWriter& video,
                const std::string& videoName,
                size_t lastWriterImageId,
                const std::string& algoName,
                const std::string& algoInfo);

    void writeFinalVideoAfter(
            const CImageBuffer& buff,
            cv::VideoWriter& video,
            size_t idsForWrite);

    virtual void onWarning(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo) override;

    void writeVideo(
            const CImageBuffer& buff,
            const std::vector<size_t>& bufferIds,
            const std::string& alertAlgorithm,
            const std::string& alertInfo,
            int newFfterAlertFrameCount);

private:
    const CFinalVideoWriterConfig& mConfig;
    std::mutex mFinalVideoMutex;

public:
    static const int sVideoType;

private: //types
    struct FinalVideo
    {
        NTypes::tMatIdType mLastImageId = 0;
        cv::VideoWriter mVideoWriter;
        std::string mDate;
    };

    struct FinalVideosMap
    {
        std::map<std::string, FinalVideo> mFinalVideoMap;
        int mAfterAlertFramesCount = -1;
    };
    std::map<size_t, FinalVideosMap> mFinalVideoMap;
};
}
