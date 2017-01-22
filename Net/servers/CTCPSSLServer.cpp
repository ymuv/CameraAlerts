#include <QFile>

#include "CClientConn.hpp"
#include "CTCPSSLServer.hpp"
#include "logging/Log.hpp"

namespace NNet
{

//https://github.com/GuiTeK/Qt-SslServer/tree/master/src - example
CTCPSSLServer::CTCPSSLServer(
        const CNetConnectionConfig& mConnectionConfig,
        const CNetCryptionConfig& mCryptionConfig,
        pWorkerProducerFunc producerFunc)
    : CTCPServer(mConnectionConfig, mCryptionConfig, producerFunc)
{

    //TODO: not done yet, only tests
    QFile certFile(QStringLiteral("/home/y/openssl/localhost.cert"));
    QFile keyFile(QStringLiteral("/home/y/openssl/localhost.key"));
    certFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);
    if (!certFile.isOpen() || !keyFile.isOpen())
    {
        LOG_WARN << "key or cert file not found" << certFile.isOpen() <<keyFile.isOpen();
        return;
    }


    mSslCertificate.reset( new QSslCertificate(&certFile, QSsl::Pem));
    mSslKey.reset(new QSslKey(&keyFile, QSsl::Rsa, QSsl::Pem));
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();

    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(*mSslCertificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1SslV3);

//    m_pWebSocketServer->setSslConfiguration(sslConfiguration);
//    this->set


    isReady = true;

}

void CTCPSSLServer::start()
{
    if (isReady)
    {
        if (!listen(QHostAddress::Any, mConnectionConfig.mPort))
        {
            LOG_WARN << "fail listen " << errorString();
        }
    }
}

void CTCPSSLServer::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket *sslSocket = new QSslSocket(this);

    sslSocket->setSocketDescriptor(socketDescriptor);
    sslSocket->setLocalCertificate(QString::fromStdString(mCryptionConfig.mLocalCertificate));
    sslSocket->setPrivateKey(QString::fromStdString(mCryptionConfig.mPrivateKey));
    sslSocket->setProtocol(QSsl::TlsV1_2);
    sslSocket->startServerEncryption();

    auto worker = mWorkerProducer();

    CClientConnection* conn = new CClientConnection(
                sslSocket, mConnectionConfig, mCryptionConfig, worker); //TODO: to shared_ptr

    worker->setClient(conn);
    connect(conn, SIGNAL(disconnected()), this, SLOT(slotClose()));
}

void CTCPSSLServer::newConnectionSlot()
{
    //not using
}
}
