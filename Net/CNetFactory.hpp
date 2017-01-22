#pragma once

#include "IWorker.hpp"

namespace NNet
{
class CNetFactory
{
public:
    CNetFactory() = delete;

    static pClient createNetClient(
            const NNet::CNetConnectionConfig& connectionConfig,
            const NNet::CNetCryptionConfig& cryptionConfig,
            pWorker worker);

    static void runMainLoop();
};

}
