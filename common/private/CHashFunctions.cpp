#include <QDataStream>

#include "alerts/zlipWrapper/CCRC.hpp"
#include "common/CConfig.hpp"
#include "common/CConnectionSettings.hpp"
#include "common/CNetProtocol.hpp"
#include "common/CHashFunctions.hpp"
#include "logging/Log.hpp"

namespace {
    /**
      size without magic number & size
    */
    const int sOffset = 8;
    const int sOffsetEnd = 12;
}

NTypes::tHash CHashFunctions::generateHash(const QByteArray& array, int posBegin, int endOffset)
{
#ifdef __WITH_ZLIB__
    if (CConfig::getInstance().mConnectionSettings->mIsCheckHash)
    {
        return CCRC::calc(array, posBegin, endOffset);
    }
#endif
    return 0;
}

bool CHashFunctions::checkHash(
        const QByteArray& inputStr,
        NTypes::tHash gettedHash,
        NTypes::tHashPosition hashPos)
{
#ifdef __WITH_ZLIB__
    if (CConfig::getInstance().mConnectionSettings->mIsCheckHash)
    {
        //test crc32
        {
            auto hashCurrent = generateHash(inputStr, sOffset, sOffsetEnd);

            if (hashPos < 0 || hashPos > inputStr.size())
            {
                LOG_WARN << "hashPos != sizeWithoutHash"
                         << "inputStr size" << inputStr.size();
                return false;
            }

            if (hashCurrent != gettedHash)
            {
                LOG_DEBUG << "hash fail getted:" << gettedHash << "; current" << hashCurrent <<"size" << inputStr.size();
                return false;
            }
        }
    }
#endif
    return true;
}

void CHashFunctions::writeHash(QByteArray& array, QDataStream& stream)
{
    NTypes::tHash hash = CHashFunctions::generateHash(array, sOffset, 0);
    stream << (NTypes::tHashPosition) array.size();
    stream << hash;
}
