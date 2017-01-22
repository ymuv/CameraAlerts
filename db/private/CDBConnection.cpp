#include "Text/CBufferData.hpp"
#include "db/CDBConfig.hpp"
#include "db/CDBConnection.hpp"
#include "logging/Log.hpp"

//Q_IMPORT_PLUGIN(qsqlpsql)


namespace
{
    const QString sSensorsTextTableName = "sensors";
    const QString sRunStatusTextTableName = "status";
}

namespace NDB
{

CDBConnection::CDBConnection(const CDBConfig& dbConfig)
    : mDbConfig(dbConfig)
{
    mDb = QSqlDatabase::addDatabase(mDbConfig.mDbType);
    mDb.setConnectOptions();
    mDb.setHostName(mDbConfig.mDbHost);
    mDb.setDatabaseName(mDbConfig.mDataBaseName);
    mDb.setUserName(mDbConfig.mUserName);
    mDb.setPassword(mDbConfig.mPassword);
    if (mDbConfig.mPort > 0)
    {
        mDb.setPort(mDbConfig.mPort);
    }

    if (mDb.open())
    {
        QSqlQuery q;

        if (!q.exec("CREATE TABLE IF NOT EXISTS " + sSensorsTextTableName+
                    "(ID  SERIAL PRIMARY KEY,"
                    " PATTERN text, "
                    " VALUE REAL,"
                    "datetime timestamp without time zone)"))
        {
            LOG_DEBUG << "fail create table " << q.lastError();
        }
        q.clear();

        if (!q.exec("CREATE TABLE IF NOT EXISTS " + sSensorsTextTableName+
                    "(ID  SERIAL PRIMARY KEY,"
                    " STATUS_HOME integer, "
                    " STATUS_MOTION integer )"))
        {
           LOG_DEBUG << "fail create table " << q.lastError();
        }
    }
    else
    {
       LOG_WARN << "fail open db" << mDbConfig.mDbHost << mDbConfig.mDataBaseName << mDb.lastError()
                << mDb.lastError().databaseText();
    }

}

CDBConnection&CDBConnection::getInstance(const CDBConfig& dbConfig)
{
    static CDBConnection instance(dbConfig);
    return instance;
}

void CDBConnection::insertTextData(const NText::CBufferData& bufferData, const QString& pattern)
{
    if (!mDb.isOpen())
    {
        return;
    }
    QSqlQuery query;
    query.prepare("INSERT INTO " +sSensorsTextTableName + " (PATTERN, VALUE, DATETIME) "
                      "VALUES (:pattern, :value, :dt)");
    query.bindValue(":pattern", pattern);
    query.bindValue(":value", bufferData.mDataCurrent);
    query.bindValue(":dt", bufferData.mDateTimeCurrent);

    std::lock_guard<std::mutex> lock(mMutex);
    if (!query.exec())
    {
        LOG_WARN << "fail add to db" << query.lastError();
    }
}

void CDBConnection::updateStatus(CRunStatus::HomeStatus status, bool motionStatus)
{
    if (!mDb.isOpen())
    {
        return;
    }
    QSqlQuery query;
    query.prepare("INSERT INTO " +sRunStatusTextTableName + " (ID, STATUS_HOME, STATUS_MOTION) "
                      "VALUES (:id, :status_home, :status_motion)");
    query.bindValue(":id", 1);
    query.bindValue(":status_home", static_cast<int>(status));
    query.bindValue(":status_motion", static_cast<int>(motionStatus));

    std::lock_guard<std::mutex> lock(mMutex);
    if (!query.exec())
    {
        LOG_WARN << "fail insert or update value in BD " << query.lastError();
    }

}


}
