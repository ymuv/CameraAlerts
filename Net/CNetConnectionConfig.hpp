#pragma once
#include <QDebug>
#include <string>

#include "Image/ImageType.hpp"
#include "Net/Types.hpp"
#include "common/NTypes.hpp"

namespace NNet
{
struct CNetConnectionConfig
{
    unsigned short mPort;
    std::string mServerHost;
    TypeProtocol mTypeProtocol;

    int mDeadlineTimer;
    size_t mImageId;
    bool mIsTextBuffer;

    NTypes::tIdType mCompressionLevel;

    ImageType::EType mRequestdImageId;
    QString mUser, mUserPassword;
};
}

//TODO: to cpp file implementation
QDebug operator<<(QDebug stream, const NNet::CNetConnectionConfig&);
