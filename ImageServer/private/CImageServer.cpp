#include "ImageServer/CImageSenderWorker.hpp"
#include "ImageServer/CImageServer.hpp"
#include "Net/CServerFactory.hpp"
#include "Net/CNetConnectionConfig.hpp"
#include "common/CConfig.hpp"
#include "common/CConnectionSettings.hpp"
#include "logging/Log.hpp"


CImageServer::CImageServer(
        const unsigned short port,
        const NNet::TypeProtocol protocol)
{
   NNet::CNetConnectionConfig connectionConfig;

   connectionConfig.mPort = port;
   connectionConfig.mDeadlineTimer = CConfig::getInstance().mConnectionSettings->mTimeOut;
   connectionConfig.mTypeProtocol = protocol;

   mpServer =  NNet::CServerFactory::createServer(
               connectionConfig,
               *CConfig::getInstance().mCryptionConfig,
               CImageSenderWorker::create);
}

void CImageServer::start()
{
    mpServer->start();
}

void CImageServer::addServer(
        const unsigned short port,
        const NNet::TypeProtocol protocol,
        std::vector<std::shared_ptr<NNet::IServer> >& servers)
{
   if (port > 0)
   {
      try
      {
         auto ptr = std::shared_ptr<NNet::IServer>(
                  new CImageServer(port, protocol));
         if (ptr)
         {
            ptr->start();
            servers.push_back(ptr);
         }
      }
      catch (...)
      {
         LOG_WARN << "fail add server";
      }
   }
}
