#pragma once

#include <QAbstractSocket>

#include "Net/INet.hpp"
#include "Net/CNetConnectionConfig.hpp"
#include "Net/CNetCryptionConfig.hpp"

namespace NNet
{
class IWorker;

class CNetBase : public INet
{
    Q_OBJECT
public:
    CNetBase(
            std::shared_ptr<NNet::IWorker> worker,
            const CNetConnectionConfig& connectionConfig,
            const CNetCryptionConfig& cryptionConfig);

    virtual void send(const QByteArray& str) override;

    virtual void setWorker(std::shared_ptr<NNet::IWorker> worker) override;

    virtual void restart() override;

    virtual std::shared_ptr<NNet::IWorker> getWorker() const override;

    void insertBeginInfo(QByteArray& strForSend, const QByteArray& str) const;

    virtual ~CNetBase();

    void onReadFinished(const QByteArray& receiveStr);
    virtual const CNetConnectionConfig&getConnectionConfig() const override;
    virtual const CNetCryptionConfig& getCryptionConfig() const override;
    virtual bool isReady() const override;

signals:
    void signalError(QAbstractSocket::SocketError, const QString);

protected:
    std::shared_ptr<IWorker> mWorker;

    CNetConnectionConfig mConnectionConfig;
    const CNetCryptionConfig& mCryptionConfig;

    QByteArray mBuffer;
    NTypes::tSizeType mSize = 0;
};
}
