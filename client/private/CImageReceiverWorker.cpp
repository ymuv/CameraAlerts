#include <thread>

#include "CDeSerializer.hpp"
#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "Image/videoInput/CInputConfig.hpp"
#include "Net/CNetValues.hpp"
#include "client/CImageReceiverWorker.hpp"
#include "common/CMainFunctions.hpp"
#include "common/CConfig.hpp"
#include "common/CConnectionSettings.hpp"
#include "common/CNetProtocol.hpp"
#include "logging/Log.hpp"

namespace
{
    const int MS_IN_SECOND = 1000;
}

CImageReceiverWorker::CImageReceiverWorker(
        const std::string& prefix,
        size_t bufferId,
        const std::string& url)
    : mAlert(prefix, url)
    , mErrorNumber(0)
    , mBufferId(bufferId)
    , mUrl(url)
{
    mTimer.setInterval(CConfig::getInstance().mConnectionSettings->mTimeOut * MS_IN_SECOND);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(timerTimeout()));

    mErrorTimer.setInterval(CConfig::getInstance().mInputConfig->mInputList[mBufferId].mTimeOut * MS_IN_SECOND);
    connect(&mErrorTimer, SIGNAL(timeout()), this, SLOT(timerErorTimeout()));

    mRequestTimer.setInterval(CMainFunctions::getSleepTime(mBufferId) * MS_IN_SECOND);
    connect(&mRequestTimer, SIGNAL(timeout()), this, SLOT(timerRequestTimeout()));

    mIncreaseErrorCountTimer.setInterval(CMainFunctions::getSleepTime(mBufferId) * MS_IN_SECOND * 2);
    connect(&mIncreaseErrorCountTimer, SIGNAL(timeout()), this, SLOT(increaseErrorCount()));

    CMainFunctions::addAlgorithm(mAlgorithms);

    mLastId = static_cast<NTypes::tMatIdType>(-1);
}

CImageReceiverWorker::~CImageReceiverWorker()
{
}

bool CImageReceiverWorker::tryLoadMat(
        const QByteArray& receiveStr,
        std::string& errString,
        std::shared_ptr<CMatWithTimeStamp> matPtr)
{
    if (CDeSerializer::load(*matPtr, receiveStr, errString, mBufferId, mLastId))
    {
        if (mpNet->getConnectionConfig().mIsTextBuffer)
        {
            CMainFunctions::doTextAlgo(mAlert);
            return true;
        }
        else
        {
            if (mLastId != static_cast<NTypes::tMatIdType>(-1) && matPtr->getId() < mLastId)
            {
                LOG_DEBUG << "server restart" << mBufferId << mpNet->getConnectionConfig();
                //TODO: test it
                //            mImageBuffer.clear(mBufferId);
                //            CMainFunctions::restartAlgo(mBufferId, mAlgorithms);
            }
            mLastId = matPtr->getId();
            if (!matPtr->getMat().empty())
            {
                CImageBuffer::getBufferInstance().push_back(mBufferId, matPtr);
                if (CConfig::getInstance().mRunStatus.mIsRunMotionAlgo)
                {
                    if (CConfig::getInstance().mIsAlgoRunAsync)
                    {
                        std::thread thread(
                                    CMainFunctions::doAlgoAsync,
                                    std::ref(mAlert),
                                    mBufferId,
                                    std::ref(mAlgorithms),
                                    std::ref(mAlgoMutex),
                                    "Net_" + mUrl);
                        thread.detach();
                    }
                    else
                    {
                        CMainFunctions::doAlgo(mAlert, mBufferId, mAlgorithms, "Net_" + mUrl);
                    }
                }
            }
            else
        {
            LOG_WARN << "empty image";
        }
        return true;
        }
    }
    return false;
}

void CImageReceiverWorker::receiveCallBack(const QByteArray& receiveStr)
{
    restartTimer(true);
    pMatWithTimeStamp matPtr;
    matPtr.reset(new CMatWithTimeStamp(
                     CConfig::getInstance().
                     mInputConfig->mInputList[mBufferId].mResizeScaleFactor));
    std::string errString;

    if (tryLoadMat(receiveStr, errString, matPtr))
    {

    }
    else
    {
        if (!CImageBuffer::getBufferInstance().empty(mBufferId))
        {
            LOG_DEBUG << "fail desialize:" << errString << "buffer size" << CImageBuffer::getBufferInstance().size(mBufferId)
                      << "\n    last timestamp:" << CImageBuffer::getBufferInstance().getLast(mBufferId)->getDateTimeWithMS()
                      << "id:" <<  CImageBuffer::getBufferInstance().getLast(mBufferId)->getId()
                      << "size:" << receiveStr.size() << "bufferId:" << mBufferId;
        }
        else
        {
            LOG_DEBUG << "fail desialize:" << errString
                      << "size:" << receiveStr.size()
                      << mpNet->getConnectionConfig()
                      << "buffer size:" << CImageBuffer::getBufferInstance().size(mBufferId);
        }
    }
    mErrorNumber = 0;
    restartTimer();

    mRequestTimer.start();
}

void CImageReceiverWorker::onConnect()
{
    int interval = mpNet->getConnectionConfig().mDeadlineTimer * MS_IN_SECOND;
    mConnectTimer.setInterval(interval);
    connect(&mConnectTimer, SIGNAL(timeout()), this, SLOT(timerConnectTimeout()));

    mConnectTimer.start(interval / 10);
}

void CImageReceiverWorker::sendCallBack(qint64)
{
    restartTimer();
}

void CImageReceiverWorker::increaseErrorCount()
{
    mIncreaseErrorCountTimer.stop();
    mErrorNumber++;
    if (mErrorNumber >= NNet::CNetValues::sMaxNetworkErrorNumber)
    {
        LOG_DEBUG << "reset connection for id" << mBufferId;
        mErrorNumber = 0;
        if (mClearCount++ >= sMaxNumberForClear)
        {
            CImageBuffer::getBufferInstance().clear(mBufferId);
            mClearCount = 0;
        }

        mErrorTimer.start();
    }
    else
    {
        sendImageRequest();
    }
}

void CImageReceiverWorker::onError(
        QAbstractSocket::SocketError sockError,
        const QString& errorDescription,
        const QString& clientDescription)
{
    LOG_INFO << errorDescription << "error id"
             << static_cast<int>(sockError)
             << "bufferId:" << mBufferId << mpNet->getConnectionConfig()
             << clientDescription;

    if (sockError == QAbstractSocket::RemoteHostClosedError
            || sockError == QAbstractSocket::ConnectionRefusedError
            || sockError == QAbstractSocket::RemoteHostClosedError
            || sockError == QAbstractSocket::SocketTimeoutError
            || sockError == QAbstractSocket::UnknownSocketError)
    {
        mErrorNumber = NNet::CNetValues::sMaxNetworkErrorNumber;
        mIncreaseErrorCountTimer.start();
    }
    else
    {
        mIncreaseErrorCountTimer.start();
    }
}

void CImageReceiverWorker::timerTimeout()
{
    LOG_DEBUG << "TimeOut" << mErrorNumber << mpNet->getConnectionConfig()
             << "bufferId" << mBufferId;
    mIncreaseErrorCountTimer.start();
//    increaseErrorCount();
}

void CImageReceiverWorker::timerErorTimeout()
{
    LOG_WARN << "timerErorTimeout; restart net for id" << mBufferId << mpNet->getConnectionConfig();
    mErrorTimer.stop();
    mpNet->restart();
    CMainFunctions::restartAlgo(mBufferId, mAlgorithms);
}

void CImageReceiverWorker::timerRequestTimeout()
{
    mRequestTimer.stop();
    sendImageRequest();
}

void CImageReceiverWorker::timerConnectTimeout()
{
    mConnectTimer.stop();
    if (mpNet->isReady())
    {
        restartTimer();
        sendImageRequest();
    }
    else
    {
        onError(QAbstractSocket::SocketTimeoutError, "timeout");
    }
}

void CImageReceiverWorker::restartTimer(bool stop)
{
    if (stop)
    {
        mTimer.stop();
    }
    else
    {
        mTimer.start();
    }
}

void CImageReceiverWorker::sendImageRequest()
{
    mTimer.start();
    mSendStr.clear();

    NTypes::tIdType imageQuality = mpNet->getConnectionConfig().mCompressionLevel;
    if (mpNet->getConnectionConfig().mIsTextBuffer)
    {
        CDeSerializer::ClientSerialize(
                    NNet::CNetProtocol::ANSVER::TEXT_BUFFER,
                    ImageType::MAX_VALUE,
                    QString::number(mpNet->getConnectionConfig().mImageId),
                    imageQuality,
                    mpNet->getConnectionConfig().mUser,
                    mpNet->getConnectionConfig().mUserPassword,
                    mSendStr);
    }
    else
    {
        CDeSerializer::ClientSerialize(
                    NNet::CNetProtocol::ANSVER::IMAGE_ID,
                    mpNet->getConnectionConfig().mRequestdImageId,
                    QString::number(mpNet->getConnectionConfig().mImageId),
                    imageQuality,
                    mpNet->getConnectionConfig().mUser,
                    mpNet->getConnectionConfig().mUserPassword,
                    mSendStr);
    }
    mpNet->send(mSendStr);
}

void CImageReceiverWorker::sendTextBufferRequest()
{
    mTimer.start();
    const auto& connectionConfig = mpNet->getConnectionConfig();

    CDeSerializer::ClientSerialize(
                NNet::CNetProtocol::ANSVER::TEXT_BUFFER,
                ImageType::MAX_VALUE,
                QString::number(mpNet->getConnectionConfig().mImageId),
                0,
                connectionConfig.mUser,
                connectionConfig.mUserPassword,
                mSendStr);
    mpNet->send(mSendStr);
}
