#include <QDataStream>

#include "Text/CBufferData.hpp"

namespace NText
{
static NTypes::tIdType  sId = 1;

CBufferData::CBufferData(NTypes::tIdType id, NText::CTextPattern::tCeserialData data, bool isAlert)
    : mDataCurrent(data)
    , mDataCurrentForCondition(data)
    , mData(data)
    , mId(id)
    , mDateTimeCreate(QDateTime::currentDateTime())
    , mDateTimeCurrent(QDateTime::currentDateTime())
    , mIsAlert(isAlert)
{
}

CBufferData::CBufferData(NText::CTextPattern::tCeserialData data, bool isAlert)
    : CBufferData(getNextId(), data, isAlert)
{
}

void CBufferData::updateData(CTextPattern::tCeserialData data)
{
    mDateTimeCurrent = QDateTime::currentDateTime();
    mDataCurrent = data;
}

NTypes::tIdType CBufferData::getNextId()
{
    return sId++;
}
}

QDataStream&operator<<(QDataStream& out, const NText::CBufferData& bufferData)
{
    out << bufferData.mId <<  bufferData.mDateTimeCreate << bufferData.mData
        << bufferData.mDateTimeCurrent << bufferData.mDataCurrent;
    return out;
}

QDataStream&operator>>(QDataStream& in, NText::CBufferData& bufferData)
{
    in >> bufferData.mId >> bufferData.mDateTimeCreate >>bufferData.mData
            >> bufferData.mDateTimeCurrent >> bufferData.mDataCurrent;
    return in;
}
