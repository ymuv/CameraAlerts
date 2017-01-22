#pragma once
#include <mutex>
#include <QLinkedList>
#include <QPair>
#include <QtNetwork>

#include "Net/IServer.hpp"
#include "Net/private/CUDPNetObject.hpp"

namespace NNet {
class CUDPServer
        : public CUDPNetObject
        , public IServer
{
    Q_OBJECT
public:
    CUDPServer(
            const CNetConnectionConfig& connectionConfig,
            const CNetCryptionConfig& cryptionConfig,
            pWorker worker);

public:
    virtual void start() override;
    virtual void sendImplementation(const QByteArray& str) override;

protected:
    virtual void afterRead(const QHostAddress& sender, quint16 senderPort) override;

private:
    using clientParam = QPair<QHostAddress, quint16>;
    QLinkedList<clientParam> mQueue;
    std::mutex mMutex;
};
}

