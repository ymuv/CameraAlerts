#pragma once
#include <string>
#include <vector>

#include "Net/IWorker.hpp"

class CClient
{
public:
    CClient() = delete;

    static void addClientToList(
            std::vector<NNet::pClient>& clientList,
            std::vector<NNet::pWorker>& workerList,
            const size_t connectionId,
            const std::string& installBinDir);
};

