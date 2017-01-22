#pragma once

#include "Net/IServer.hpp"
#include "Net/Types.hpp"

namespace NMJPEG
{
struct CMJPEGServerConfig;

class CMJPEGFactory
{
public:
    CMJPEGFactory() = delete;

    static std::shared_ptr<NNet::IServer> createMJPEGServer(
            const CMJPEGServerConfig& config,
            const NNet::TypeProtocol protocol = NNet::TypeProtocol::TCP);
};
}
