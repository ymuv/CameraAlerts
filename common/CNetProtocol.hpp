#pragma once

#include <QDataStream>

#include "common/NTypes.hpp"

namespace NNet
{
class CNetProtocol
{
public:
    enum class ANSVER
    {
        IMAGE_ID = 0,
        BUFFER_SIZE = 1,
        CONFIG = 2,
        CODE_PARSE_FAIL = 3,
        BUFFER_INDEX_OUT_OF_RANGE = 4,
        BUFFER_EMPTY = 5,
        MAT_NULL_PTR = 6,
        IMAGE_TYPE_OUT_OF_RANGE = 7,
        TEXT_BUFFER = 8,
        COMMAND = 9,

        NOT_IMPLEMENTED,

        MAX_VALUE //must be last
    };

    CNetProtocol() = delete;
};
}

