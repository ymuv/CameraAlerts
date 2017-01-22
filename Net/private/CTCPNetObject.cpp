#include <QHostAddress>

#include "CTCPNetObject.hpp"
#include "logging/Log.hpp"

void NNet::CTCPNetObject::slotReadClient()
{
    QByteArray readStr;
    readStr = mpSocket->readAll();
    onReadFinished(readStr);
}

void NNet::CTCPNetObject::slotError(QAbstractSocket::SocketError error)
{
    LOG_DEBUG << "error:" << error << mpSocket->errorString();

    if (mWorker)
    {
        mWorker->onError(error, mpSocket->errorString(),
                         "IP:" + mpSocket->peerAddress().toString() + "" +
                         mpSocket->peerName() + ";Port:" + QString::number(mpSocket->peerPort()));
    }
}

void NNet::CTCPNetObject::slotWriteFinish(qint64 size)
{
    mWorker->sendCallBack(size);
}

NNet::CTCPNetObject::CTCPNetObject(
        const NNet::CNetConnectionConfig& connectionConfig,
        const NNet::CNetCryptionConfig& cryptionConfig,
        NNet::pWorker worker,
        QAbstractSocket* socket)
    : CNetBase(worker, connectionConfig, cryptionConfig)
    , mpSocket(socket)
{
    if (mpSocket)
    {
        mpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    }
}

void NNet::CTCPNetObject::start()
{
    mWorker->onConnect();
}

void NNet::CTCPNetObject::sendImplementation(const QByteArray& str)
{
    QByteArray sendStr;
    insertBeginInfo(sendStr, str);
    mpSocket->write(sendStr);
}

void NNet::CTCPNetObject::close()
{
    mpSocket->close();
    mpSocket->disconnectFromHost();
    if (mpSocket->isOpen())
    {
        static const int DISCONNECT_TIME = 1;
        //TODO: do async
        if (!mpSocket->waitForDisconnected(DISCONNECT_TIME))
        {
            LOG_DEBUG << "waitForDisconnected fail" << mpSocket->errorString();
        }
    }
    delete mpSocket;
    mpSocket = nullptr;
}
