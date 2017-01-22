#include "CTCPClient.hpp"

namespace NNet
{
CTCPClient::CTCPClient(
        const CNetConnectionConfig& connectionConfig,
        const CNetCryptionConfig& cryptionConfig,
        pWorker worker)
    : CTCPNetObject(connectionConfig, cryptionConfig, worker)
{
}

void CTCPClient::start()
{
    mpSocket = new QTcpSocket();
    QString host =QString::fromUtf8(mConnectionConfig.mServerHost.c_str());
    mpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    mpSocket->connectToHost(host, mConnectionConfig.mPort);
    connect(mpSocket, SIGNAL(readyRead()),
            this, SLOT(slotReadClient()));

    connect(mpSocket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(slotWriteFinish(qint64)));
    connect(mpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError)));
    mWorker->onConnect();
}

bool CTCPClient::isReady() const
{
    return mpSocket->state() == QAbstractSocket::ConnectedState;
}

}
