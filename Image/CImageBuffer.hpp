#pragma once

#include <memory>
#include <mutex>
#include <vector>

template <typename T>
class QList;
class QDateTime;

class CMatWithTimeStamp;
using pMatWithTimeStamp=std::shared_ptr<CMatWithTimeStamp>;

class CImageBuffer
{
private:
    class CRingBuffer;
public:
    static CImageBuffer& getBufferInstance();
    void push_back(size_t bufferId, const pMatWithTimeStamp matPtr);

    bool isBufferExistAndFull(size_t bufferId) const;
    bool isBufferExistAndNotEmpty(size_t bufferId) const;

    const pMatWithTimeStamp getLast(size_t bufferId) const;
    const pMatWithTimeStamp getLast2(size_t bufferId) const;

    bool isFull(size_t bufferId) const;

    bool getHasAlert(size_t bufferId) const;
    void setHasAlert(size_t bufferId, bool value);
    const QDateTime& getLastAlertDt(size_t bufferId) const;

    size_t size(size_t bufferId) const;
    size_t longSize(size_t bufferId) const;
    bool empty(size_t bufferId) const;
    int maxSize(size_t bufferId) const;
    void clear(size_t bufferId);

    size_t getNextId(size_t bufferId) const;

    const QList<pMatWithTimeStamp>& getList(size_t bufferId) const;
    const QList<pMatWithTimeStamp>& getLongList(size_t bufferId) const;
    const QList<pMatWithTimeStamp> getCopy(size_t bufferId) const;
    const QList<pMatWithTimeStamp> getLongCopy(size_t bufferId) const;
    size_t getBuffersSize() const;

    void resizeBuffersToSize(
            size_t newSize,
            int bufferSize,
            int longBufferSize,
            int longBufferSkipCount = 10);

    const std::string& getBufferInfo(size_t bufferId) const;
    void setBufferInfo(size_t bufferId, const std::string& str);

    CImageBuffer(const CImageBuffer& other);

private: //func
    CImageBuffer();

private:
    std::vector<std::shared_ptr<CRingBuffer>> mBuffersVector;
    mutable std::mutex mMutex;
};
using matPtrLists = QList<pMatWithTimeStamp>;
