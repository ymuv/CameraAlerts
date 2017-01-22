#pragma once
#include <QString>

#include "common/NTypes.hpp"

class CHashFunctions
{
public:
    CHashFunctions() = delete;

    static bool checkHash(
            const QByteArray& inputStr,
            NTypes::tHash gettedHash,
            NTypes::tHashPosition hashPos);

    static void writeHash(QByteArray& array, QDataStream& stream);

private:
    static NTypes::tHash generateHash(const QByteArray& array, int posBegin, int endOffset);
};

