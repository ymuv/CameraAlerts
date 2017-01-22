#include "CUDPClient.hpp"

NNet::CUDPClient::CUDPClient(
        const CNetConnectionConfig& connectionConfig,
        const CNetCryptionConfig& cryptionConfig,
        pWorker worker)
    : CUDPNetObject(connectionConfig, cryptionConfig, worker)
{
}

void NNet::CUDPClient::start()
{
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    mWorker->onConnect();
}

void NNet::CUDPClient::sendImplementation(const QByteArray& str)
{
    QByteArray sendStr;
    QHostAddress servAddress(QString::fromStdString(mConnectionConfig.mServerHost));

    insertBeginInfo(sendStr, str);
    write(servAddress, mConnectionConfig.mPort, sendStr);
}
