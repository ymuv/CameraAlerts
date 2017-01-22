#pragma once
#include "Net/IServer.hpp"
#include "MJPEGServer/CMJPEGServerConfig.hpp"

#include <QObject>
#include <QTcpServer>
#include <QTimer>
namespace NMJPEG
{
class CImageServerMJPEG
        : public QTcpServer
        , public NNet::IServer
{
    Q_OBJECT
public:
    explicit CImageServerMJPEG(const CMJPEGServerConfig& config);
    virtual ~CImageServerMJPEG();

   virtual void start();// override;
public slots:
    void newConnectionSlot();
    void txRx();
    void closingClient();

private:
    QTcpServer* server ;
    const CMJPEGServerConfig& mConfig;
};
}
