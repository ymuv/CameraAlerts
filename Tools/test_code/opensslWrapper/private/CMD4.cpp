#include <string>
#include <QByteArray>
#include <zlib.h>
#include <openssl/md4.h>

#include "zlib/CMD4.hpp"
#include "logging/Log.hpp"

//void calcMD5(const QByteArray& data, int offset, int endOffset)
template <class T>
T CMD4::calcMD4(const T& data, int offset, int endOffset)
{
    if ((data.size()- offset - endOffset) > 0)
    {
        char md4digest[MD4_DIGEST_LENGTH];
        MD4(reinterpret_cast<const unsigned char *>( data.data() + offset),
            data.size()- offset - endOffset,
            reinterpret_cast<unsigned char *>(md4digest));
        T retValue(md4digest,MD4_DIGEST_LENGTH );
        return retValue;
    }
    return QByteArray();
}

template QByteArray CMD4::calcMD4(
    const QByteArray&, int, int);
