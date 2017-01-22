#pragma once

#include <QTcpSocket>
#include <memory>

#include "Net/private/CTCPNetObject.hpp"

namespace NNet
{
class CTCPClient : public CTCPNetObject
{

public:
    CTCPClient(
            const NNet::CNetConnectionConfig& connectionConfig,
            const NNet::CNetCryptionConfig& cryptionConfig,
            pWorker worker);
    virtual void start() override;

    virtual bool isReady() const override;
};
}

