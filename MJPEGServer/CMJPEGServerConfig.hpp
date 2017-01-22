#pragma once
#include <string>

namespace NMJPEG
{
struct CMJPEGServerConfig
{
    unsigned short mPort;
    int mJPEGQuality;
    int mRefreshTimeMS; //ms

    std::string mUserName;
    std::string mPassword;
};
}
