#pragma once

#include <vector>
#include <string>

namespace NAlarms
{
struct CFinalVideoWriterConfig
{
    std::vector<std::string> mSaveFinalVideoPaths;
    int mFrameBeforeAlertToVideo;
    int mFrameAfterAlertToVideo;
    bool mIsWriteMaskToFinalVideo = true;
    double mVideoFps;
};
}
