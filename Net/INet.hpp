#pragma once

#include <QObject>
#include <memory>

namespace NNet
{

struct CNetConnectionConfig;
struct CNetCryptionConfig;
class IWorker;

class INet : public QObject
{
public:
    virtual void start() = 0;
    virtual void close() = 0;
    virtual void restart() = 0;

    virtual void send(const QByteArray& str) = 0;

    virtual void sendImplementation(const QByteArray& str) = 0;

    virtual const CNetConnectionConfig& getConnectionConfig() const = 0;
    virtual const CNetCryptionConfig& getCryptionConfig() const = 0;

    virtual std::shared_ptr<NNet::IWorker> getWorker() const = 0;
    virtual void setWorker(std::shared_ptr<NNet::IWorker> worker) = 0;

    virtual bool isReady() const = 0;
};
using pClient = std::shared_ptr<INet>;
}
