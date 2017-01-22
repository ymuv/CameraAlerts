#include "Net/IWorker.hpp"

namespace NNet
{
void IWorker::slotOnError(
        QAbstractSocket::SocketError error,
        const QString& errorDescription)
{
    onError(error, errorDescription, "unknown error");
}

}
