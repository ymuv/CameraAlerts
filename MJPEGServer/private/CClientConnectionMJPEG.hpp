#include <QObject>
#include <QTcpServer>
#include <QTimer>

namespace NMJPEG
{
struct CMJPEGServerConfig;

class CClientConnectionMJPEG : public QObject
{
    Q_OBJECT
public:
    CClientConnectionMJPEG(QTcpSocket *sock, const CMJPEGServerConfig& config);
    void writeImage();
    ~CClientConnectionMJPEG();

private:
    QByteArray addImgToAll();
private:
    QTcpSocket* mSocket;
    QTimer mTimerNextImage;
    const CMJPEGServerConfig& mConfig;
    bool mIsLoginValid;
    size_t mBufferId;
    size_t mImageType;
    std::vector<uchar> mBuff;


protected slots:
    void slotClose();
    void slotReadClient();
    void slotError(QAbstractSocket::SocketError error);
    void timerTimeout();
signals:
    void disconnected();
};
}
