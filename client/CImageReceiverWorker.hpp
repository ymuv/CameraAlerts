#pragma once
#include <QObject>
#include <QTimer>
#include <mutex>

#include "Image/motionAlgorithms/IAlertAlgorithm.hpp"
#include "Net/CWorkerBase.hpp"
#include "common/CAlertNotification.hpp"
#include "common/NTypes.hpp"

//class CMatWithTimeStamp;

class CImageReceiverWorker
        : public NNet::CWorkerBase
{
    Q_OBJECT
public:
    CImageReceiverWorker(
            const std::string& prefix,
            size_t bufferId,
            const std::string& url);
    virtual ~CImageReceiverWorker();

    virtual void receiveCallBack(const QByteArray& receiveStr) override;
    virtual void onConnect() override;
    virtual void sendCallBack(qint64) override;

    virtual void onError(
            QAbstractSocket::SocketError sockError,
            const QString& errorDescription,
            const QString& clientDescription = "") override;

private slots:
    void timerTimeout();
    void timerErorTimeout();
    void timerRequestTimeout();
    void timerConnectTimeout();
    void increaseErrorCount();

private:
    void restartTimer(bool stop = false);
    void sendImageRequest();
    void sendTextBufferRequest();
    void sendConfigRequest();
    void sendImageBufferSizeRequest();

    bool tryLoadMat(
            const QByteArray& receiveStr,
            std::string& errString,
            std::shared_ptr<CMatWithTimeStamp> matPtr);

private:
    QTimer mTimer;

    //this timer restart NNet. If restart net in onError, can be stack overflow, if problem repeat.
    QTimer mErrorTimer;
    QTimer mRequestTimer;
    QTimer mConnectTimer;
    QTimer mIncreaseErrorCountTimer;

    CAlertNotification mAlert;

    algorighmListPtr mAlgorithms;
    size_t mErrorNumber;

    size_t mBufferId = 0;
    NTypes::tMatIdType mLastId;

    std::string mUrl;
    std::mutex mAlgoMutex;

    cv::Mat mMask;
    QByteArray mSendStr;
    int mClearCount = 0;
    static const int sMaxNumberForClear = 10;
};
