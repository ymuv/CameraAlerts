#pragma once
#include <QDebug>
#include <QString>
#include <vector>

#include "common/NTypes.hpp"

namespace NText
{
struct CTextPattern
{
    using tCeserialData = NTypes::tCeserialData;
    QString mPattern;
    tCeserialData mNormalValue;
    tCeserialData mTreshholdAlert;
    tCeserialData mTreshholdWarning;
    bool mIsReverse; //if true - alert if currentValue <= tresh, else if currentValue >= tresh
    bool mIsProcess;
    float mDiffNotSave; //if (newValue - oldValue) >= mDiffNotSave - save;
    std::vector<size_t> mBuffersForWrite;
    int mMaxBufferLastSize = 10;
    bool mIsAddToDB;
};

struct CTextPatterns
{
    std::vector<CTextPattern> mPatterns;
};
}
QDebug operator<<(QDebug stream, const NText::CTextPatterns&);
