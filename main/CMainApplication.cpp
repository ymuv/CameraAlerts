#include <thread>

#ifdef __linux__
#include <signal.h>
#endif

#include "Image/videoInput/CVideoCapture.hpp"
#include "Image/videoInput/CInputConfig.hpp"
#include "MJPEGServer/CMJPEGFactory.hpp"
#include "common/CAlertNotification.hpp"
#include "common/CConfig.hpp"
#include "common/CConnectionSettings.hpp"
#include "common/CMainFunctions.hpp"
#include "db/CDBConnection.hpp"
#include "logging/Log.hpp"
#include "webDownload/CWebDownloader.hpp"
#include "webDownload/CWebDownloaderConfig.hpp"

#include "Net/CServerFactory.hpp"
#ifdef __BUILD_SERVERS__
#include "ImageServer/CImageServer.hpp"
#endif

#ifdef __BUILD_CLIENTS__
#include "client/CClient.hpp"
#endif

#ifdef __WITH_SERIAL__
#include "serial/CSerialPorts.hpp"
#endif

void loop(NVideoInput::CVideoCapture& videoCapture,
          std::vector<std::shared_ptr<NAlgorithms::IAlertAlgorithm>>& algorithms)
{
    std::vector<std::shared_ptr<CAlertNotification>> alerts(videoCapture.getBufferSizeWithoutConnection());
    for (size_t i = 0; i < videoCapture.getBufferSizeWithoutConnection(); i++)
    {
        alerts[i].reset(new CAlertNotification(i));
    }
    while (true)
    {
        double timeBeging = (double) cv::getTickCount();
        for (size_t bufferId = 0;
             bufferId < videoCapture.getBufferSizeWithoutConnection();
             bufferId++)
        {
            if (videoCapture.processNextFrame(bufferId))
            {
#ifdef __BUILD_MOTION_ALGO__
                if (CConfig::getInstance().mRunStatus.mIsRunMotionAlgo)
                {
                    CMainFunctions::doAlgo( *alerts[bufferId], bufferId, algorithms);
                }
#endif
            }
        }
        CMainFunctions::sleepTo(timeBeging);
        cv::waitKey(1);
    }
}
void loopOne(
        NVideoInput::CVideoCapture& videoCapture,
        size_t bufferId,
        std::vector<std::shared_ptr<NAlgorithms::IAlertAlgorithm>>& algorithms)
{
    CAlertNotification alert(bufferId);
    std::mutex mutex;

    while (true)
    {
        double timeBeging = (double) cv::getTickCount();
        if (videoCapture.processNextFrame(bufferId) &&
                CConfig::getInstance().mRunStatus.mIsRunMotionAlgo)
        {
#ifdef __BUILD_MOTION_ALGO__
            if (CConfig::getInstance().mIsAlgoRunAsync)
            {
                std::string str;
                std::thread thread(
                            CMainFunctions::doAlgoAsync,
                            std::ref(alert),
                            bufferId,
                            std::ref(algorithms),
                            std::ref(mutex),
                            str);
                thread.detach();
            }
            else
            {
                CMainFunctions::doAlgo(alert, bufferId, algorithms);
            }
#endif
        }
        CMainFunctions::sleepTo(timeBeging, bufferId);
    }
}

int main(int argc, char** argv)
{
//    signal(SIGSEGV, CMainFunctions::printTrace);
//    signal(SIGABRT, CMainFunctions::printTrace);
    CMainFunctions::parseParams(argc, argv, __INSTALL_BIN_DIR__);
    CConfig& config = CConfig::getInstance();

    NVideoInput::CVideoCapture videoCapture(*config.mInputConfig);
    LOG_DEBUG << "videoCapture created";

    std::vector<std::shared_ptr<NNet::IServer>> servers;

#ifdef __BUILD_MJPEG__
    auto ptr = NMJPEG::CMJPEGFactory::createMJPEGServer(*config.mMJPEGServerConfig);
    if (ptr)
    {
        servers.push_back(ptr);
    }
#endif

#ifdef __BUILD_SERVERS__

    NDB::CDBConnection::getInstance(*CConfig::getInstance().mDataBaseConfig);


    CImageServer::addServer(config.mConnectionSettings->mTCPPort,
                            NNet::TypeProtocol::TCP, servers);
    CImageServer::addServer(config.mConnectionSettings->mUDPPort,
                            NNet::TypeProtocol::UDP, servers);

//    CImageServer::addServer (10000, ImageType::IMAGE_RGB_RESIZED,
//                             TypeProtocol::TCP_SSL, servers);
#endif
//    size_t bufferSizeWithoutConnection = videoCapture.getBufferSizeWithoutConnection();

#ifdef __BUILD_CLIENTS__
//    size_t connectionSize = config.mInputConfig->mConnectionList.size();
//    CImageBuffer::getBufferInstance().resizeBuffersToSize(
//                connectionSize + bufferSizeWithoutConnection);

    std::vector<NNet::pWorker> workerList;
    std::vector<NNet::pClient> clientList;

    for (size_t connectionId = 0;
         connectionId < config.mInputConfig->mInputList.size();
         connectionId++)
    {
        if (config.mInputConfig->mInputList[connectionId].mType == NVideoInput::InputType::URL)
        {
            CClient::addClientToList(
                        clientList, workerList,
                        connectionId,
                        "");
        }
    }
#endif

    std::shared_ptr<NWebDownwloader::CWebDownloader> webDownloader;
    if (!CConfig::getInstance().mWebDownloaders->mWebDownloaders.empty())
    {
        webDownloader.reset(new NWebDownwloader::CWebDownloader(
                                *CConfig::getInstance().mWebDownloaders));
    }

#ifdef __WITH_SERIAL__
    for (const auto& serialConfig : config.mSerialPortsConfig)
    {
        NSerial::CSerialPorts::getInstance().addPort(*serialConfig);
    }
#endif

    std::vector<std::shared_ptr<NAlgorithms::IAlertAlgorithm>> algorithms;
#ifdef __BUILD_VIDEOCAPTURE__
    CMainFunctions::addAlgorithm(algorithms);
#ifdef __BUILD_SERVERS__
    std::vector<std::thread> buffers(videoCapture.getBufferSizeWithoutConnection());

    for (size_t bufferId = 0; bufferId < videoCapture.getBufferSizeWithoutConnection(); bufferId++)
    {
        buffers[bufferId] = std::thread (loopOne,
                                         std::ref(videoCapture),
                                         bufferId,
                                         std::ref(algorithms)
                                         );
    }
    NNet::CServerFactory::runMainLoop();
    buffers[0].join();
#else //__BUILD_SERVERS__
    loop(videoCapture, algorithms);
#endif //__BUILD_SERVERS__


#else //__BUILD_VIDEOCAPTURE__
    NNet::CServerFactory::runMainLoop();
#endif //__BUILD_VIDEOCAPTURE__

    return EXIT_SUCCESS;
}
