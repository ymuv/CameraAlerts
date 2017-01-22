#pragma once

#include <QDateTime>

#include "Text/CTextPatterns.hpp"
#include "common/NTypes.hpp"


namespace NText
{
/**
 * @brief The CBufferData struct data in text buffer
 */
struct CBufferData
{
public: //static methods
    static NTypes::tIdType getNextId();

public: //functions
    CBufferData() {}
    explicit CBufferData(NText::CTextPattern::tCeserialData data, bool isAlert );
    CBufferData(NTypes::tIdType id, NText::CTextPattern::tCeserialData data,  bool isAlert);
    void updateData(CTextPattern::tCeserialData data);

public: //members
    CTextPattern::tCeserialData mDataCurrent;
    CTextPattern::tCeserialData mDataCurrentForCondition; //if no new data long time (CAlertsStatus::sMaxSeconds), set to noram value if is alert
    NText::CTextPattern::tCeserialData mData;
    NTypes::tIdType mId;


    QDateTime mDateTimeCreate;
    QDateTime mDateTimeCurrent;
    bool mIsAlert = false;
};
}

QDataStream& operator<<(QDataStream &out, const NText::CBufferData& bufferData);
QDataStream& operator>>(QDataStream &in, NText::CBufferData& bufferData);
