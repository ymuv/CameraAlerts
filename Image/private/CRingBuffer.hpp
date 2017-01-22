#pragma once

#include <QDateTime>
#include <QList>

#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "Image/motionAlgorithms/IAlertAlgorithm.hpp"

class CImageBuffer::CRingBuffer
{
public:
    CRingBuffer(int size, int longSize, int longSkipCount, const std::string& info);
    ~CRingBuffer();

    bool isFull() const;

    void push_back(const pMatWithTimeStamp elem);

    int capacity() const;

    bool empty() const;

    void clear();

    int size() const;
    int longSize() const;

    size_t getNextId();
    void setHasAlert(bool alertStatus);
    bool getHasAlert() const;
    const QDateTime& getLastAlertDt() const;

    const QList<pMatWithTimeStamp>& getList() const;
    const QList<pMatWithTimeStamp>& getLongList() const;
    const pMatWithTimeStamp operator[](int elementId) const;
    const std::string& getInfo() const;


    CRingBuffer(const CRingBuffer& other);

    void setInfoStr(const std::string& infoStr);

private:
    QList<pMatWithTimeStamp> mList;
    QList<pMatWithTimeStamp> mLongList;
    int mMaxSize;
    int mMaxLongSize;
    int mLongSkipCount;
    std::string mInfoStr;
    size_t mId;
    size_t mLongId;
    mutable std::mutex mMutex;
    bool mIsHasAlert = false;

    QDateTime mLastAlertDt;
};
