#pragma once

#include "IServer.hpp"
#include "IWorker.hpp"

namespace NNet
{
class CServerFactory
{
public:
    CServerFactory() = delete;

    static pServer createServer(
            const CNetConnectionConfig& connConfig,
            const CNetCryptionConfig& cryptConfig,
            pWorkerProducerFunc workerProducer);

    static void runMainLoop();
};
}
