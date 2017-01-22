#include <string>
#include <QByteArray>
#include <zlib.h>

#include "alerts/zlipWrapper/CCRC.hpp"

template <class T>
unsigned long CCRC::calc(const T& data, int offset, int endOffset)
{
    unsigned long  crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc,
                reinterpret_cast<const unsigned char *>( data.data() + offset),
                data.size()- offset - endOffset);
    return  crc;
}

template unsigned long CCRC::calc(
    const QByteArray&, int, int);

