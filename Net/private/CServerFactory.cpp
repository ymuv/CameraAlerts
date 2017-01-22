#include "Net/CServerFactory.hpp"
#include "Net/servers/CTCPServer.hpp"
#include "Net/servers/CTCPSSLServer.hpp"
#include "Net/servers/CUDPServer.hpp"
#include "logging/Log.hpp"

namespace NNet
{
namespace
{
int i = 0;
static QCoreApplication app(i, nullptr);
static bool isRun = false;
void run()
{
    if (!isRun)
    {
        isRun = true;
        app.exec();
    }
}
}

pServer CServerFactory::createServer(
      const CNetConnectionConfig& connConfig,
      const CNetCryptionConfig& cryptConfig,
      pWorkerProducerFunc workerProducer)
{
   if (connConfig.mTypeProtocol == TypeProtocol::TCP)
   {
       return std::shared_ptr<IServer>(
                   new CTCPServer(
                       connConfig,
                       cryptConfig,
                       workerProducer));
   }
   else if (connConfig.mTypeProtocol == TypeProtocol::UDP)
   {
       pWorker worker = workerProducer();
       auto server =  std::shared_ptr<CUDPServer>(
                   new CUDPServer(connConfig,
                                  cryptConfig,
                                  worker));
       pServer serv = server;
       worker->setClient(server.get());

       return serv;
   }
   else if (connConfig.mTypeProtocol == TypeProtocol::TCP_SSL)
   {
       return std::shared_ptr<IServer>(
                   new CTCPSSLServer(
                       connConfig,
                       cryptConfig,
                       workerProducer));
   }
   else {
      LOG_ERROR << "not implemented type";
      return nullptr;
   }

}

void NNet::CServerFactory::runMainLoop()
{
    run();
}
}
