#pragma once

#include <QMap>
#include <QString>

namespace NCommand
{
struct CCommand
{
    QString mUrl;
    QString mSerialPort;
    QString mSerialCommand;
    QString mSay;
};

struct CCommandsMap
{
    QMap<QString, CCommand> mCommandMap;
};

}
