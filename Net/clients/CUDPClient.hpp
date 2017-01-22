#pragma once

#include "Net/private/CUDPNetObject.hpp"
namespace NNet
{
class CUDPClient : public CUDPNetObject
{
public:
    CUDPClient(
            const NNet::CNetConnectionConfig& connectionConfig,
            const NNet::CNetCryptionConfig& cryptionConfig,
            pWorker worker);

    virtual void start() override;

    virtual void sendImplementation(const QByteArray& str) override;
};
}
