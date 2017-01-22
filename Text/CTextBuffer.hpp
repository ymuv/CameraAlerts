#pragma once

#include <QDataStream>
#include <QList>
#include <QMap>

#include "Text/CBufferData.hpp"

namespace NText
{
class CTextBuffer
{
public:
    using tListType = QList<CBufferData>; //list of buffers data
    using tMapType = QMap<QString, tListType>; //key - pattern, value - - QList(datetime, value)

    using tMapLastIdByPatternType = QMap<QString, NTypes::tIdType>; //pattern, last id
    using tMapLastProcessedIdType = QMap<QString, NTypes::tIdType>; //pattern, last id

public:
    void append(
            const NText::CTextPattern& pattern,
            NText::CTextPattern::tCeserialData data,
            bool isAlert = false);

public: //static
    static CTextBuffer& getInstance();

    tMapType& getMap() ;
    void setMap(const tMapType& map);
    void setList(const QString& pattern, const tListType& patternList);

    void addToList(
            NText::CTextPattern::tCeserialData data,
            tListType& list,
            int maxListSize,
            NText::CTextPattern::tCeserialData diffNotSave,
            const QString& pattern,
            bool isAlert);

    bool isExistPatternInLastProcessedMap(const QString& pattern) const;
    void setValueToLastProcessedMap(const QString& pattern, NTypes::tIdType lastId);
    NTypes::tIdType getLastId(const QString& pattern) const;

private:
    CTextBuffer();
    void addToDB(const CBufferData& data, const NText::CTextPattern& pattern) const;

private: //methods
    tMapType mMap;
    int mMaxBufferLastSize = 50;
};
}
