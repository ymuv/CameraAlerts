#pragma once
#include <string>

namespace NNet
{
struct CNetCryptionConfig
{
    std::string mKey;
    size_t mKeySize;

    std::string mKey2;
    size_t mKey2Size;

    std::string mKey3;
    size_t mKey3Size;

    std::string mInitialVector;
    //for tcp_ssl
    std::string mLocalCertificate;
    std::string mPrivateKey;

    bool mIsUseCryption;

    const static size_t sBlockSize = 16; //for aes it always 16
};
}
