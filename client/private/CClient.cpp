#include <signal.h>

#include "Image/CImageBuffer.hpp"
#include "Image/videoInput/CInputConfig.hpp"
#include "Net/CNetFactory.hpp"
#include "client/CClient.hpp"
#include "client/CImageReceiverWorker.hpp"
#include "common/CConfig.hpp"
#include "common/CConnectionSettings.hpp"
#include "logging/Log.hpp"

void CClient::addClientToList(
      std::vector<NNet::pClient>& clientList,
      std::vector<NNet::pWorker>& workerList,
      const size_t connectionId,
      const std::string& installBinDir)
{
    CConfig& config = CConfig::getInstance();
    NNet::CNetConnectionConfig& connectionConfig = config.mInputConfig->mInputList[connectionId].
            mConnectionConfig;

    connectionConfig.mDeadlineTimer = config.mConnectionSettings->mTimeOut;

    LOG_INFO << connectionConfig;

    std::string URL = NNet::toString(connectionConfig.mTypeProtocol)
            + "_" + connectionConfig.mServerHost + "_" + std::to_string(connectionConfig.mPort);
    if (!connectionConfig.mIsTextBuffer)
    {
        URL += "_image_id" + std::to_string(static_cast<int>(connectionConfig.mRequestdImageId))
            +  "_id" + std::to_string(connectionConfig.mImageId);
    }
    else
    {
        URL += "_textBuffer";
    }
    size_t bufferOldSize = CImageBuffer::getBufferInstance().getBuffersSize();

    CImageBuffer::getBufferInstance().resizeBuffersToSize(
                bufferOldSize +1,
                config.mInputConfig->mInputList[connectionId].mBufferSize,
                config.mInputConfig->mInputList[connectionId].mLongBufferSize,
                config.mInputConfig->mInputList[connectionId].mLongBufferSkip);

    CImageBuffer::getBufferInstance().setBufferInfo(bufferOldSize , URL);
    NNet::pWorker worker(new CImageReceiverWorker(
                             installBinDir,
                             bufferOldSize,
                             URL));

    auto client = NNet::CNetFactory::createNetClient(
                connectionConfig,
                *CConfig::getInstance().mCryptionConfig,
                worker);

    workerList.push_back(worker);
    clientList.push_back(client);

    worker->setClient(client.get());
    client->start();
}
