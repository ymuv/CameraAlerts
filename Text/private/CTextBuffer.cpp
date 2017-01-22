#include "Text/CTextBuffer.hpp"
#include "common/CConfig.hpp"
#include "db/CDBConnection.hpp"
#include "db/CDBConfig.hpp"

namespace NText
{

void CTextBuffer::append(
        const CTextPattern& pattern,
        CTextPattern::tCeserialData data,
        bool isAlert)
{
    auto& list = mMap[pattern.mPattern];

    bool isAdded = false;
    if (list.empty())
    {
        CBufferData dataForAppend(data, isAlert);
        list.append(dataForAppend);
        isAdded = true;
        addToDB(dataForAppend, pattern);
    }
    else if (fabs(list.last().mData - data) > std::numeric_limits<double>::epsilon())
    {
        if (pattern.mDiffNotSave  < std::numeric_limits<NText::CTextPattern::tCeserialData>::epsilon())
        {
            CBufferData dataForAppend(data, isAlert);
            list.append(dataForAppend);
            isAdded = true;
            addToDB(dataForAppend, pattern);
        }
        else if (fabs(list.last().mData - data) >= pattern.mDiffNotSave)
        {
            CBufferData dataForAppend(data, isAlert);
            list.append(dataForAppend);
            isAdded = true;
            addToDB(dataForAppend, pattern);
        }
    }
    if (!isAdded && isAlert && !list.empty())
    {
        list.last().mId = CBufferData::getNextId();
        list.last().updateData(data);
    }
    else
    {
        //TODO: not tested yet
        list.last().updateData(data);
    }

    if (list.size() > mMaxBufferLastSize)
    {
        list.pop_front();
    }
}

CTextBuffer&CTextBuffer::getInstance()
{
    static CTextBuffer buffer;
    return buffer;
}

CTextBuffer::CTextBuffer()
{
}

void CTextBuffer::addToDB(const CBufferData& data, const CTextPattern& pattern) const
{
#ifdef __WITH_DB__
        if (CConfig::getInstance().mDataBaseConfig->isFull() && pattern.mIsAddToDB)
        {
            NDB::CDBConnection::getInstance(*CConfig::getInstance().mDataBaseConfig).
                    insertTextData(data, pattern.mPattern);
        }
#endif
}

CTextBuffer::tMapType& CTextBuffer::getMap()
{
    return mMap;
}

void CTextBuffer::setMap(const CTextBuffer::tMapType& map)
{
    mMap = map;
}

void CTextBuffer::setList(const QString& pattern, const CTextBuffer::tListType& patternList)
{
    mMap[pattern] = patternList;
}
}
