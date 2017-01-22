#include "CClientConn.hpp"
#include "logging/Log.hpp"

namespace NNet
{
CClientConnection::CClientConnection(
      QTcpSocket* sock,
      const CNetConnectionConfig& connectionConfig,
      const CNetCryptionConfig& cryptionConfig,
      pWorker worker)
    : CTCPNetObject(connectionConfig, cryptionConfig, worker, sock)
{
    connect(mpSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
    connect(mpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotWriteFinish(qint64)));
    connect(mpSocket, SIGNAL(disconnected()), this, SLOT(slotClose()));
    connect(mpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError)));
    start();
}

void CClientConnection::slotClose()
{
    LOG_DEBUG << "slot close";
    emit disconnected();
}

CClientConnection::~CClientConnection()
{
    if (mpSocket)
    {
        mpSocket->close();
    }
}
void CClientConnection::restart()
{
}
}
