#pragma once
#include <vector>

#include "Net/IServer.hpp"
#include "Net/Types.hpp"

class CImageServer : public NNet::IServer
{
public: //static
    //TODO: move to factory class and return ptr
    static void addServer(
            const unsigned short port,
            const NNet::TypeProtocol protocol,
            std::vector<std::shared_ptr<NNet::IServer>>& servers);

public:
    CImageServer(
            const unsigned short port,
            const NNet::TypeProtocol protocol);

    virtual void start() override;

private:
   NNet::pServer mpServer;
};

