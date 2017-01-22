#pragma once
#include <QDebug>
#include <QString>
#include <string>
#include <vector>

#include "alerts/CAlertCoefConfig.hpp"

namespace NWebDownwloader
{
struct CWebDownloaderConfig
{
    QString mUrl;
    //TODO: bad position here
    int mRefreshTimeMS;
};

struct CWebDownloaderConfigList
{
    std::vector<CWebDownloaderConfig> mWebDownloaders;
};


}
