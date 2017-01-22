#include "webDownload/CWebDownloader.hpp"
#include "webDownload/CWebDownloaderConfig.hpp"
#include "common/CMainFunctions.hpp"
#include "logging/Log.hpp"

namespace NWebDownwloader
{

CWebDownloader::CWebDownloader(const CWebDownloaderConfigList& config)
    : mConfig(config)
{
    connect(&mWebCtrl, SIGNAL (finished(QNetworkReply*)),
            this, SLOT (webDownloaded(QNetworkReply*)));
    mRequestTimer.setInterval(mConfig.mWebDownloaders.front().mRefreshTimeMS);
    connect(&mRequestTimer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    mRequestTimer.start();
}

void CWebDownloader::downloadUrl(const QString& url)
{
    QUrl urlClass(url);
    QNetworkRequest request(urlClass);
    mWebCtrl.get(request);
}

void CWebDownloader::webDownloaded(QNetworkReply* pReply)
{
    if (pReply->error() == QNetworkReply::NetworkError::NoError)
    {
        mByteArray = pReply->readAll();
        pReply->deleteLater();

        if (!mByteArray.isEmpty())
        {
//            CAlertNotification alert(*CConfig::getInstance().mAlertNotificationConfig,  "web Config");
//            CMainFunctions::doTextAlgo(mByteArray, alert);
        }
    }
    else
    {
        LOG_WARN << "error: " << pReply->error() << pReply->errorString() << pReply->url().toString();
    }
}

void CWebDownloader::timerTimeout()
{
    for (const auto& downloadConfig : mConfig.mWebDownloaders)
    {
        downloadUrl(downloadConfig.mUrl);
    }

}


}
