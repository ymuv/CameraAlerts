#pragma once

#include "Net/CWorkerBase.hpp"
#include "common/CNetProtocol.hpp"

class CImageSenderWorker : public NNet::CWorkerBase
{
public: //static
    static NNet::pWorker create();

public:
    CImageSenderWorker();

    virtual void receiveCallBack(const QByteArray& receiveStr) override;
    virtual void sendCallBack(qint64) override;

    virtual void onError(
            QAbstractSocket::SocketError sockError,
            const QString& errorDescription,
            const QString& clientDescription) override;

    virtual ~CImageSenderWorker();

private:
    NNet::CNetProtocol::ANSVER parse(
            NNet::CNetProtocol::ANSVER ansver,
            const QString& str2,
            size_t& bufferId);
public:
    virtual void onConnect() override;
};

