#pragma once
#include <QByteArray>
#include <QObject>
#include <QNetworkReply>
#include <QTimer>

namespace NWebDownwloader
{
struct CWebDownloaderConfigList;

class CWebDownloader : public QObject
{
    Q_OBJECT
public:
    explicit CWebDownloader(const CWebDownloaderConfigList& config);
    void downloadUrl(const QString& url);

private slots:
    void webDownloaded(QNetworkReply* pReply);
    void timerTimeout();

private:
    const CWebDownloaderConfigList& mConfig;
    QByteArray mByteArray;
    QNetworkAccessManager mWebCtrl;
    QTimer mRequestTimer;
};
}

