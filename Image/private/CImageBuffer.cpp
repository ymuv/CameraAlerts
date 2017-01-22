#include <assert.h>
#include <QDateTime>

#include "CRingBuffer.hpp"
#include "Image/CImageBuffer.hpp"
#include "logging/Log.hpp"

namespace
{
    const static int sMaxSeconds = 100;
}

const pMatWithTimeStamp CImageBuffer::getLast(size_t bufferId) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    //TODO: remove assert!
    assert(bufferId < mBuffersVector.size());
    auto size =  mBuffersVector[bufferId]->size();
    return (*mBuffersVector[bufferId])[size - 1];
}

const pMatWithTimeStamp CImageBuffer::getLast2(size_t bufferId) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto size =  mBuffersVector[bufferId]->size();
    return (*mBuffersVector[bufferId])[size - 2];
}

CImageBuffer& CImageBuffer::getBufferInstance()
{
    static CImageBuffer bufferInstance;
    return bufferInstance;
}

CImageBuffer::CImageBuffer()
{
}

void CImageBuffer::resizeBuffersToSize(
        size_t newSize,
        int bufferSize,
        int longBufferSize,
        int longBufferSkipCount)
{
    std::lock_guard<std::mutex> lock(mMutex);
    for (size_t i = mBuffersVector.size(); i < newSize; i++)
    {
        std::shared_ptr<CRingBuffer> ptr(
                    new CRingBuffer(
                        bufferSize,
                        longBufferSize,
                        longBufferSkipCount,
                        std::to_string(i)));
        mBuffersVector.push_back(ptr);
    }
}

const std::string& CImageBuffer::getBufferInfo(size_t bufferId) const
{
    if (mBuffersVector.size() <= bufferId)
    {
        static const std::string emptyStr = "error buffer";
        return emptyStr;
    }
    return mBuffersVector[bufferId]->getInfo();
}

void CImageBuffer::setBufferInfo(size_t bufferId, const std::string& str)
{
    mBuffersVector[bufferId]->setInfoStr(str);
}

CImageBuffer::CImageBuffer(const CImageBuffer& other)
{
    if (this != &other)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        std::lock_guard<std::mutex> lockOther(other.mMutex);
        for (const auto buffer : other.mBuffersVector)
        {
            mBuffersVector.push_back(std::make_shared<CImageBuffer::CRingBuffer>(*buffer));
        }
    }
}

void CImageBuffer::push_back(size_t bufferId, const pMatWithTimeStamp matPtr)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mBuffersVector[bufferId]->push_back(matPtr);
}

bool CImageBuffer::isBufferExistAndFull(size_t bufferId) const
{
    if (getBuffersSize() <= bufferId)
    {
        LOG_DEBUG << "buff.getBuffersSize() >= bufferId" << getBuffersSize() <<  bufferId;
        return false;
    }
    if (!isFull(bufferId))
    {
        LOG_DEBUG << "buff.empty(bufferId)";
        return false;
    }
    if (!getLast(bufferId))
    {
        LOG_DEBUG << "!buff.getLast(bufferId)";
        return false;
    }
    return true;
}

bool CImageBuffer::isBufferExistAndNotEmpty(size_t bufferId) const
{
    if (getBuffersSize() <= bufferId)
    {
        LOG_DEBUG << "buff.getBuffersSize() >= bufferId" << getBuffersSize() <<  bufferId;
        return false;
    }
    if (empty(bufferId))
    {
        LOG_DEBUG << "buff.empty(bufferId)";
        return false;
    }
    if (!getLast(bufferId))
    {
        LOG_DEBUG << "!buff.getLast(bufferId)";
        return false;
    }
    return true;
}

bool CImageBuffer::isFull(size_t bufferId) const
{
    return mBuffersVector[bufferId]->isFull();
}

bool CImageBuffer::getHasAlert(size_t bufferId) const
{
    if (!mBuffersVector[bufferId]->isFull()
            || mBuffersVector[bufferId]->size() <= 1
            || !mBuffersVector[bufferId]->getHasAlert())
    {
        return false;
    }

    if (QDateTime::currentDateTime().secsTo(getLast(bufferId)->getDateTimeCreation()) > sMaxSeconds)
    {
        LOG_WARN << "diff is too big";
        mBuffersVector[bufferId]->setHasAlert(false);
    }

    return mBuffersVector[bufferId]->getHasAlert();
}

void CImageBuffer::setHasAlert(size_t bufferId, bool value)
{
    mBuffersVector[bufferId]->setHasAlert(value);
}

const QDateTime&CImageBuffer::getLastAlertDt(size_t bufferId) const
{
    return mBuffersVector[bufferId]->getLastAlertDt();
}

size_t CImageBuffer::size(size_t bufferId) const
{
    return static_cast<size_t>(mBuffersVector[bufferId]->size());
}

size_t CImageBuffer::longSize(size_t bufferId) const
{
    return static_cast<size_t>(mBuffersVector[bufferId]->longSize());
}

bool CImageBuffer::empty(size_t bufferId) const
{
    return mBuffersVector[bufferId]->empty();
}

int CImageBuffer::maxSize(size_t bufferId) const
{
    return mBuffersVector[bufferId]->capacity();
}

void CImageBuffer::clear(size_t bufferId)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mBuffersVector[bufferId]->clear();
}

size_t CImageBuffer::getNextId(size_t bufferId) const
{
    return mBuffersVector[bufferId]->getNextId();
}

const QList<pMatWithTimeStamp>&CImageBuffer::getList(size_t bufferId) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mBuffersVector[bufferId]->getList();
}

const QList<pMatWithTimeStamp>&CImageBuffer::getLongList(size_t bufferId) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mBuffersVector[bufferId]->getLongList();
}

const QList<pMatWithTimeStamp> CImageBuffer::getCopy(size_t bufferId) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mBuffersVector[bufferId]->getList();
}

const QList<pMatWithTimeStamp> CImageBuffer::getLongCopy(size_t bufferId) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mBuffersVector[bufferId]->getLongList();
}

size_t CImageBuffer::getBuffersSize() const
{
    return mBuffersVector.size();
}

