#pragma once

#include <string>
#include <memory>
#include <QAbstractSocket>
#include <QByteArray>
#include <QString>

#include "INet.hpp"

namespace NNet
{
class IWorker : public QObject
{
    Q_OBJECT
public:
    virtual void receiveCallBack(const QByteArray& receiveStr) = 0;

    virtual void onConnect() = 0;
    virtual void sendCallBack(qint64 size) = 0;

    virtual void onError(
            QAbstractSocket::SocketError sockError,
            const QString& errorDescription,
            const QString& clientDescription) = 0;

    virtual void setClient(INet* client) = 0;
    virtual ~IWorker()
    {}

public slots:
    void slotOnError(QAbstractSocket::SocketError error, const QString& errorDescription);
};

using pWorker = std::shared_ptr<IWorker>;
using pWorkerProducerFunc = pWorker (*)();

}
