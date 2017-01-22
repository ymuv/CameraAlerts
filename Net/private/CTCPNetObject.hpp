#pragma once
#include <QAbstractSocket>

#include "Net/IWorker.hpp"
#include "Net/private/CNetBase.hpp"

namespace NNet
{
class CTCPNetObject : public CNetBase
{
    Q_OBJECT
public:
    CTCPNetObject(
            const CNetConnectionConfig& connectionConfig,
            const CNetCryptionConfig& cryptionConfig,
            pWorker worker,
            QAbstractSocket* socket = nullptr);

    virtual void start() override;
    virtual void sendImplementation(const QByteArray& str) override;
    virtual void close() override;

protected slots:
    void slotReadClient();
    void slotError(QAbstractSocket::SocketError error);
    void slotWriteFinish(qint64 size);

protected:
    //TODO: shared_ptr?
    QAbstractSocket* mpSocket;
};

} //namespace NNet
