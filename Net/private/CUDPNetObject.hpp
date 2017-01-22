#pragma once
#include <QtNetwork>

#include "Net/IWorker.hpp"
#include "Net/private/CNetBase.hpp"

namespace NNet
{
class CUDPNetObject
        : public CNetBase
{
    Q_OBJECT
public: //methods
    CUDPNetObject(
            const CNetConnectionConfig& connectionConfig,
            const CNetCryptionConfig& cryptionConfig,
            pWorker worker);

    virtual void close() override;

protected slots:
    void readyRead();

protected:  //methods
    void write(
            const QHostAddress& sender,
            quint16 senderPort,
            const std::string& str);

    void write(
            const QHostAddress& sender,
            quint16 senderPort,
            const QByteArray& str);

    virtual void afterRead(const QHostAddress& sender, quint16 senderPort);

protected: //fields
//    pWorker mWorker;
    QUdpSocket* mSocket;
};

} //namespace NNet
