#include <QTcpSocket>

#include "CClientConnectionMJPEG.hpp"
#include "CImageServerMJPEG.hpp"
#include "logging/Log.hpp"
namespace NMJPEG
{
void CImageServerMJPEG::start()
{
    if(!server->listen(QHostAddress::Any, mConfig.mPort))
    {
        LOG_WARN << "fail start server on port " << mConfig.mPort;
    }
}

void CImageServerMJPEG::newConnectionSlot()
{
    QTcpSocket* socket = server->nextPendingConnection();
    CClientConnectionMJPEG* conn = new CClientConnectionMJPEG(socket, mConfig);
    connect(conn, SIGNAL(disconnected()), this, SLOT(closingClient()));
}

void CImageServerMJPEG::txRx()
{

}

void CImageServerMJPEG::closingClient()
{
    CClientConnectionMJPEG* clientSocket = static_cast<CClientConnectionMJPEG*>(sender());
    if (clientSocket)
    {
        delete clientSocket;
        clientSocket = nullptr;
    }
}

CImageServerMJPEG::CImageServerMJPEG(const CMJPEGServerConfig& config)
    : mConfig(config)
{
    server = new QTcpServer();
    connect(server, SIGNAL(newConnection()),this, SLOT(newConnectionSlot()));
}

CImageServerMJPEG::~CImageServerMJPEG()
{

}
}
