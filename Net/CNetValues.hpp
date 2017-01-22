#pragma once

#include <QDataStream>

#include "common/NTypes.hpp"

namespace NNet
{
class CNetValues
{
public:
    const static QDataStream::Version sDataStreamVersion = QDataStream::Qt_5_0;

    const static NTypes::tVersionType sVersion  = 1;
    static const NTypes::tMagicNumberType sMagicNumber = 0xA0B0C0D0;

    //after this errors image buffer clear and restart connection
    const static size_t sMaxNetworkErrorNumber = 8;

    CNetValues() = delete;
};

}
