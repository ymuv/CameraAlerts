#pragma once

#include <string>

namespace NNet
{

enum class TypeProtocol
{
    TCP = 0,
    UDP = 1,
    TCP_SSL = 2,

    MAX_VALUE = 3
};
}

namespace NNet
{
    std::string toString(TypeProtocol type);
}
