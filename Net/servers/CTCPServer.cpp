#include <QtNetwork>
#include <QCoreApplication>
#include <thread>

#include "CClientConn.hpp"
#include "logging/Log.hpp"
#include "Net/servers/CTCPServer.hpp"


NNet::CTCPServer::CTCPServer(
        const CNetConnectionConfig& connectionConfig,
        const CNetCryptionConfig& cryptionConfig,
        pWorkerProducerFunc producerFunc)
    : mConnectionConfig(connectionConfig)
    , mCryptionConfig(cryptionConfig)
    , mWorkerProducer(producerFunc)
{
}

void NNet::CTCPServer::start()
{
    if (!listen(QHostAddress::Any, mConnectionConfig.mPort))
    {
       LOG_WARN << "fail listen tcp server on port " <<  mConnectionConfig.mPort << errorString();
    }
    connect(this, SIGNAL(newConnection()), this, SLOT(newConnectionSlot()));
}

NNet::CTCPServer::~CTCPServer()
{
}

void NNet::CTCPServer::newConnectionSlot()
{
    QTcpSocket* clientSocket = nextPendingConnection();
    auto worker = mWorkerProducer();

    CClientConnection* conn =
          new CClientConnection(clientSocket, mConnectionConfig, mCryptionConfig, worker); //TODO: to shared_ptr
    worker->setClient(conn); //BAD
    connect(conn, SIGNAL(disconnected()), this, SLOT(slotClose()));
}

void NNet::CTCPServer::slotClose()
{
    CClientConnection* clientSocket = static_cast<CClientConnection*>(sender());
    delete clientSocket;
    clientSocket = nullptr;
}
