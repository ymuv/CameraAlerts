#pragma once
#include <QSslConfiguration>
#include <QSslSocket>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>

#include "CTCPServer.hpp"

namespace NNet
{
class CTCPSSLServer : public CTCPServer
{
public:
    CTCPSSLServer(
            const CNetConnectionConfig& mConnectionConfig,
            const CNetCryptionConfig& mCryptionConfig,
            pWorkerProducerFunc producerFunc);

    virtual void start() override;

    virtual void incomingConnection(qintptr handle) override;

public slots:
   void newConnectionSlot() override;

private:
    bool isReady = false;
    QSslConfiguration sslConfiguration;

    std::shared_ptr<QSslCertificate> mSslCertificate;
    std::shared_ptr<QSslKey> mSslKey;

};

}
