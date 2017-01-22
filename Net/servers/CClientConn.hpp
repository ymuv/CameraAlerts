#pragma once
#include <QTcpSocket>
#include <memory>

#include "Net/private/CTCPNetObject.hpp"
namespace NNet
{
class CClientConnection : public NNet::CTCPNetObject
{
    Q_OBJECT
public:
    CClientConnection(
            QTcpSocket *sock,
            const CNetConnectionConfig& mConnectionConfig,
            const CNetCryptionConfig& mCryptionConfig,
            pWorker worker);

    virtual ~CClientConnection();

    virtual void restart() override;
private slots:
    void slotClose();

signals:
    void disconnected();
};
}
