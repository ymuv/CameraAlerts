#include "Net/Types.hpp"

namespace NNet
{
    std::string toString(TypeProtocol type)
    {
        static_assert(static_cast<int>(TypeProtocol::MAX_VALUE) == 3, "max value != 3, fix it");
        return (type == TypeProtocol::UDP)
                ? "UDP" : (type == TypeProtocol::TCP) ? "TCP"  : "TCP_SSL";
    }
}
