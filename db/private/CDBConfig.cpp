#include "db/CDBConfig.hpp"

#define NOT_EMPTY(qstr) (!qstr.isEmpty())
bool NDB::CDBConfig::isFull() const
{
    if (NOT_EMPTY(mPassword) && NOT_EMPTY(mDbHost) && NOT_EMPTY(mDataBaseName)
            && NOT_EMPTY(mDbType))
    {
        return true;
    }
    return false;
}
