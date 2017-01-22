#include "CImageServerMJPEG.hpp"
#include "MJPEGServer/CMJPEGFactory.hpp"
#include "logging/Log.hpp"

std::shared_ptr<NNet::IServer> NMJPEG::CMJPEGFactory::createMJPEGServer(
        const NMJPEG::CMJPEGServerConfig& config,
        const NNet::TypeProtocol) //TODO:  tcp, tcp/ssl
{
    if (config.mPort> 0)
    {
        try
        {
            auto ptr = std::shared_ptr<NNet::IServer>(new CImageServerMJPEG(config));
            if (ptr)
            {
                ptr->start();
                return ptr;
            }
        }
        catch (...)
        {
            LOG_WARN << "fail add server";
        }
    }
    return nullptr;
}
