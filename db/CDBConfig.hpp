#pragma once
#include <QString>
namespace NDB
{
struct CDBConfig
{
    QString mUserName;
    QString mPassword;
    QString mDbHost;
    QString mDataBaseName;
    QString mDbType;
    int mPort;

    bool isFull() const;
};
}
