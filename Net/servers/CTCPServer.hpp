#pragma once
#include <QTcpServer>

#include "Net/IServer.hpp"
#include "Net/IWorker.hpp"

#include "Net/CNetConnectionConfig.hpp"

namespace NNet {

class CTCPServer
        : public QTcpServer
        , public IServer
{
    Q_OBJECT
public:
    CTCPServer(
            const CNetConnectionConfig& mConnectionConfig,
            const CNetCryptionConfig& mCryptionConfig,
            pWorkerProducerFunc);

    virtual void start() override;

    virtual ~CTCPServer();

protected:
    const CNetConnectionConfig mConnectionConfig;
    const CNetCryptionConfig& mCryptionConfig;

    pWorkerProducerFunc mWorkerProducer;

public slots:
    virtual void newConnectionSlot();
    void slotClose();
};

}

