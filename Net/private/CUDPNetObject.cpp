#include "CUDPNetObject.hpp"

NNet::CUDPNetObject::CUDPNetObject(
      const NNet::CNetConnectionConfig& connectionConfig,
      const NNet::CNetCryptionConfig& cryptionConfig,
      NNet::pWorker worker)
    : CNetBase(
          worker,
          connectionConfig,
          cryptionConfig)
    , mSocket(new QUdpSocket(this))
{

    mWorker = worker;
}

void NNet::CUDPNetObject::readyRead()
{
    QByteArray readStr;
    readStr.resize(mSocket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    mSocket->readDatagram(readStr.data(), readStr.size(),
                          &sender, &senderPort);

    afterRead(sender, senderPort);

    onReadFinished(readStr);
}

void NNet::CUDPNetObject::write(
        const QHostAddress& sender,
        quint16 senderPort,
        const QByteArray& str)
{
    auto result = mSocket->writeDatagram(str, sender, senderPort);
    if (mWorker)
    {
        if (result > 0)
            mWorker->sendCallBack(result);
        else
        {
            mWorker->onError(
                        QAbstractSocket::DatagramTooLargeError, mSocket->errorString(),
                        "IP:" + mSocket->peerAddress().toString() + "" +
                        mSocket->peerName() + ";Port:" + QString::number(mSocket->peerPort()));
        }
    }
}

void NNet::CUDPNetObject::afterRead(const QHostAddress& sender, quint16 senderPort)
{
    //nothing
}


void NNet::CUDPNetObject::close()
{
    mSocket->close();
}
