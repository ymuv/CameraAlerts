#include "CUDPServer.hpp"
#include "logging/Log.hpp"

NNet::CUDPServer::CUDPServer(
      const NNet::CNetConnectionConfig& connectionConfig,
      const NNet::CNetCryptionConfig& cryptionConfig,
      NNet::pWorker worker)
: CUDPNetObject(connectionConfig, cryptionConfig, worker)
{
}

void NNet::CUDPServer::start()
{
    if (!mSocket->bind(QHostAddress::Any, mConnectionConfig.mPort))
    {
       LOG_INFO << "fail bind" << mSocket->errorString();
    }
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void NNet::CUDPServer::sendImplementation(const QByteArray& str)
{
   std::lock_guard<std::mutex> lock(mMutex);

   if (!mQueue.empty())
    {
        clientParam param = mQueue.first();
        mQueue.removeFirst();

        QByteArray strForSend;
        insertBeginInfo(strForSend, str);

        write(param.first, param.second, strForSend);
    }
}

void NNet::CUDPServer::afterRead(const QHostAddress& sender, quint16 senderPort)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mQueue.append(clientParam(sender, senderPort));
}
