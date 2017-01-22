#pragma once
#include <QtSql/QtSql>
#include <memory>
#include <mutex>

#include "common/CRunStatus.hpp"

class QSqlDatabase;

namespace NText
{
    struct CBufferData;
}
namespace NDB
{
struct CDBConfig;

class CDBConnection
{
public:
    static CDBConnection& getInstance(const CDBConfig& dbConfig);
public:
    void insertTextData(const NText::CBufferData& bufferData, const QString& pattern);
    void updateStatus(CRunStatus::HomeStatus status, bool motionStatus);

private:
    explicit CDBConnection(const CDBConfig& dbConfig);

private:
    const CDBConfig& mDbConfig;
    QSqlDatabase mDb;
    std::mutex mMutex;
};
}

