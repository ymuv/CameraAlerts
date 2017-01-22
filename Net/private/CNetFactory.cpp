#include "Net/CNetFactory.hpp"
#include "Net/CServerFactory.hpp"
#include "Net/clients/CTCPClient.hpp"
#include "Net/clients/CUDPClient.hpp"
#include "logging/Log.hpp"

namespace NNet
{
namespace
{
pClient createNetObjInternal(
        const NNet::CNetConnectionConfig& connectionConfig,
        const NNet::CNetCryptionConfig& cryptionConfig,
        pWorker worker)
{
    if (connectionConfig.mTypeProtocol == TypeProtocol::UDP)
    {
        return std::make_shared<CUDPClient>(connectionConfig, cryptionConfig, worker);

    }
    else if (connectionConfig.mTypeProtocol == TypeProtocol::TCP)
    {
        return std::make_shared<CTCPClient>(connectionConfig, cryptionConfig, worker);
    }
    else
    {
       LOG_ERROR << "not implemented type";
       return nullptr;
    }
}
} //anonymous namespace


pClient CNetFactory::createNetClient(
        const CNetConnectionConfig& connectionConfig,
        const CNetCryptionConfig& cryptionConfig,
        pWorker worker)
{
    auto client =  createNetObjInternal(
                connectionConfig, cryptionConfig, worker);
    worker->setClient(client.get());
    return client;
}

void CNetFactory::runMainLoop()
{
    CServerFactory::runMainLoop();
}
}

QDebug operator<<(QDebug stream, const NNet::CNetConnectionConfig& config)
{
    stream << (QString("CConnectionConfig:[")
               +"ServerHost=" + QString::fromStdString(config.mServerHost) + "; "
               + "Port=" + QString::number(config.mPort) + "; "
               + "ImageId=" + QString::number(config.mImageId) + "; "
               + "Type=" + QString::number(config.mRequestdImageId) + "; "
               + "IsTextBuffer=" + QString::number(static_cast<int>(config.mIsTextBuffer)) + "; "
               + QString::fromStdString(NNet::toString(config.mTypeProtocol)) + "]");
    return stream;
}
