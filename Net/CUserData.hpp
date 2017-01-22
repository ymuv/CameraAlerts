#pragma once
#include <QString>
#include <QMap>

namespace NNet
{
struct CUserData
{
    QString mPass;
};

using tAllowedUsers = QMap<QString, CUserData>;
}
