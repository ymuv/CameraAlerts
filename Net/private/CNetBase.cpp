#include "Net/AES/CAES.hpp"
#include "Net/CNetValues.hpp"
#include "Net/IWorker.hpp"
#include "Net/private/CNetBase.hpp"
#include "logging/Log.hpp"

namespace NNet
{

static const int sizeOfBegin = sizeof(quint32) + sizeof(int);

CNetBase::CNetBase(
        std::shared_ptr<IWorker> worker,
        const CNetConnectionConfig& connectionConfig,
        const CNetCryptionConfig& cryptionConfig)
    : mWorker(worker)
    , mConnectionConfig(connectionConfig)
    , mCryptionConfig(cryptionConfig)
{
    if (mWorker)
    {
        connect(this, SIGNAL(signalError(QAbstractSocket::SocketError, const QString)),
                worker.get(), SLOT(slotOnError(QAbstractSocket::SocketError, const QString)));
    }
}

void CNetBase::send(const QByteArray& str)
{
    CAES::crypt(str, this);
}

void CNetBase::onReadFinished(const QByteArray& receiveStr)
{
    QDataStream stream(receiveStr);
    stream.setVersion(CNetValues::sDataStreamVersion);
    NTypes::tMagicNumberType mn;
    stream >> mn;
    if (mn == CNetValues::sMagicNumber)
    {
        stream >> mSize;
        if (mSize > 0)
        {
            if (mSize != receiveStr.size())
            {
                mBuffer.reserve(mSize);
                mBuffer.clear();
                mBuffer = receiveStr.mid(sizeOfBegin);
            }
            else
            {
                mBuffer.reserve(mSize);
                mBuffer.clear();
                mBuffer = receiveStr.mid(sizeOfBegin);
                CAES::decrypt(mBuffer, this);
            }
        }
    }
    else
    {
        mBuffer.append(receiveStr);
        if (mBuffer.size() == (mSize - sizeOfBegin))
        {
            CAES::decrypt(mBuffer, this);
        } //TODO: Dublicate data in sender (mistake).  Rebuild all servers and remove
        else if (mBuffer.size() > (mSize - sizeOfBegin))
        {
            LOG_DEBUG << "Warning!!!!";
            CAES::decrypt(mBuffer.mid(0, mSize), this);
        }
    }
}

const CNetConnectionConfig& CNetBase::getConnectionConfig() const
{
    return mConnectionConfig;
}

const CNetCryptionConfig&CNetBase::getCryptionConfig() const
{
    return mCryptionConfig;
}

bool CNetBase::isReady() const
{
    return true;
}

CNetBase::~CNetBase()
{
}

//TODO: call sendImpl here
void CNetBase::insertBeginInfo(QByteArray& strForSend, const QByteArray& str) const
{
    strForSend.reserve(sizeOfBegin + str.length());
    {
        QDataStream stream(&strForSend, QIODevice::ReadWrite);
        stream.setVersion(CNetValues::sDataStreamVersion);
        stream << CNetValues::sMagicNumber;
        stream << (sizeOfBegin + str.length());
    }
    strForSend.append(str);
}

void CNetBase::setWorker(std::shared_ptr<IWorker> worker)
{
    mWorker = worker;
}

void CNetBase::restart()
{
    close();
    start();
}

std::shared_ptr<IWorker> CNetBase::getWorker() const
{
    return mWorker;
}
}
