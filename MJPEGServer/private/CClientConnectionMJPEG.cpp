#include <opencv2/highgui/highgui.hpp>
#include <QRegExp>
#include <QUrlQuery>
#include <QTcpSocket>

#include "CClientConnectionMJPEG.hpp"
#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "MJPEGServer/CMJPEGServerConfig.hpp"
#include "Text/CTextBuffer.hpp"
#include "common/CMainFunctions.hpp"
#include "common/CMemoryUssage.hpp"
#include "logging/Log.hpp"

namespace NMJPEG
{
namespace
{
    const QString sDateTimeWithMSToStrFormat = "yyyy-MM-dd_hh-mm-ss.zzz";
}

CClientConnectionMJPEG::CClientConnectionMJPEG(
        QTcpSocket* sock,
        const CMJPEGServerConfig& config)
    : mSocket(sock)
    , mConfig(config)
{
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(slotClose()));
    connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError)));
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));

    mTimerNextImage.setInterval(mConfig.mRefreshTimeMS);
    connect(&mTimerNextImage, SIGNAL(timeout()), this, SLOT(timerTimeout()));
}

CClientConnectionMJPEG::~CClientConnectionMJPEG()
{
}

void CClientConnectionMJPEG::slotClose()
{
    emit disconnected();
}

void CClientConnectionMJPEG::writeImage()
{
    if (!mSocket->isOpen())
    {
        LOG_WARN << "not open; FIX IT";
        return ;
    }

    if (CImageBuffer::getBufferInstance().getBuffersSize() > 0
            && CImageBuffer::getBufferInstance().getBuffersSize() > mBufferId)
    {
        if (CImageBuffer::getBufferInstance().size(mBufferId) > 0)
        {
            {
                pMatWithTimeStamp matPtr = CImageBuffer::getBufferInstance().getLast(mBufferId);
                if (matPtr)
                {
                    CMatWithTimeStampLocker locker(matPtr);
                    ImageType::EType type = static_cast<ImageType::EType>(mImageType);
                    const cv::Mat mat = matPtr->getMat(type);

                    std::vector<int> compressionParams;
                    compressionParams.push_back(CV_IMWRITE_JPEG_QUALITY);
                    compressionParams.push_back(mConfig.mJPEGQuality);

                    imencode(".jpg", mat, mBuff, compressionParams);
                }
            }

            QByteArray BoundaryString = ("--boundary\r\n" \
                                         "Content-Type: image/jpeg\r\n" \
                                         "Content-Length: ");
            BoundaryString.append(QString::number(mBuff.size()));
            BoundaryString.append("\r\n\r\n");
            mSocket->write(BoundaryString);
            mSocket->write(reinterpret_cast<const char*>(&mBuff.at(0)), mBuff.size());
        }
        else
        {
            LOG_WARN << "empty";
        }
    }
    else
    {
        LOG_WARN << "empty";
    }
}

void CClientConnectionMJPEG::slotReadClient()
{

    auto data = mSocket->readAll();
    bool ok = true;

    QString receivedStr(data);
    bool isAll = false;


    for (const auto& str : receivedStr.split(QRegExp("[\r\n][\r\n]*")))
    {
        if (str.startsWith("GET"))
        {
            QStringList list = str.split(" ");
            if (list.size() >= 2)
            {
                QString fullPath = list[1];
                if (fullPath.startsWith("/all"))
                {
                    isAll = true;
                }
                QUrlQuery query(fullPath);
                mIsLoginValid = true;
                if (!mConfig.mUserName.empty() && !mConfig.mPassword.empty())
                {
                    //TODO: do simply
                    auto user = query.queryItemValue("user").replace("?","").replace("/", "").toStdString();
                    auto pass = query.queryItemValue("password").replace("?","").replace("/", "").toStdString();
                    if (user == mConfig.mUserName && pass == mConfig.mPassword)
                    {
                        mIsLoginValid = true;
                    }
                    else
                    {
                        mIsLoginValid = false;
                    }
                }

                mBufferId = 0;
                mImageType = ImageType::IMAGE_RGB_FULL;
                QStringList paths = fullPath.split("/", QString::SkipEmptyParts);
                if (paths.size() >= 1)
                {
                    paths[0].toUInt(&ok);
                    if (ok)
                        mBufferId = paths[0].toUInt(&ok);
                }
                if (paths.size() >= 2)
                {
                    paths[1].toInt(&ok);
                    if(ok)
                        mImageType = paths[1].toUInt(&ok);
                }
            }
        }
    }
    if (!mIsLoginValid)
    {
        QByteArray ContentType = ("HTTP/1.0 200 OK\r\n" );
        mSocket->write(ContentType);
        mSocket->close();
    }
    if (isAll)
    {
        QByteArray ContentType = ("HTTP/1.0 200 OK\r\n<html>"\
                                  "Server: en.code-bude.net example server\r\n" \
                                  "Cache-Control: no-cache\r\n" \
                                  "Cache-Control: private\r\n" \
                                  "Content-Type: text/html;charset=utf-8\r\n\r\n"
                                  "<meta http-equiv=\"refresh\" content=\"30\" >"
                                  "<body>");

        ContentType += addImgToAll();
        ContentType += "<br>Run at: " + QString::fromStdString(CMainFunctions::sRunTime) + "<br>";
        ContentType += "<br>Mem: curr:" + QString::number(CMemoryUssage::processMemoryUssage())
                + " max:" +  QString::number(CMemoryUssage::processMaxMemoryUssage()) + "<br>";

        const auto& map = NText::CTextBuffer::getInstance().getMap();

        ContentType.append("<hr>");
        ContentType.append("<hr>");
        for (const auto& patternKey :  map.keys()) //tMapPatternType
        {
            ContentType.append("<hr>");
            {
                ContentType.append("<br>");
                ContentType.append(" ");
                ContentType.append(patternKey);
                ContentType.append(" ");
                ContentType.append("<br><table border=\"1\">");
                for (const auto& bufferData : map[patternKey]) //tListType
                {
                    if (bufferData.mIsAlert)
                    {
                        ContentType.append("<tr bgcolor=red>");
                    }
                    else
                    {
                        ContentType.append("<tr>");
                    }

                    ContentType.append("<td>");
                    ContentType.append(QString::number(bufferData.mId));
                    ContentType.append("</td>");
                    ContentType.append("<td>");
                    ContentType.append(bufferData.mDateTimeCreate.toString(sDateTimeWithMSToStrFormat));
                    ContentType.append("</td>");

                    ContentType.append("<td>");
                    ContentType.append(QString::number(bufferData.mData));
                    ContentType.append("</td>");

                    ContentType.append("<td>");
                    ContentType.append(bufferData.mDateTimeCurrent.toString(sDateTimeWithMSToStrFormat));
                    ContentType.append("</td>");

                    ContentType.append("<td>");
                    ContentType.append(QString::number(bufferData.mDataCurrent));
                    ContentType.append("</td>");

                    ContentType.append("</tr>");
                }
                ContentType.append("</table>");
            }
        }

        ContentType += "</body></html>";
        mSocket->write(ContentType);
        mSocket->close();
    }
    else
    {
        if (mBufferId < CImageBuffer::getBufferInstance().getBuffersSize() && mImageType < ImageType::MAX_VALUE)
        {
            QByteArray ContentType = ("HTTP/1.0 200 OK\r\n" \
                                      "Server: en.code-bude.net example server\r\n" \
                                      "Cache-Control: no-cache\r\n" \
                                      "Cache-Control: private\r\n" \
                                      "Content-Type: multipart/x-mixed-replace;boundary=--boundary\r\n\r\n");
            mSocket->write(ContentType);
            mTimerNextImage.start();
        }
        else
        {
            QString errStr = "Request fail. Request buffer id " + QString::number(mBufferId)
                    + " and image type " + QString::number(mImageType)
                    + "; but max buffer id = " + QString::number(CImageBuffer::getBufferInstance().getBuffersSize())
                    + " max image type = " + QString::number(ImageType::MAX_VALUE-1);
            mSocket->write(errStr.toLatin1());
            mSocket->close();
        }
    }
}

QByteArray CClientConnectionMJPEG::addImgToAll()
{
    QByteArray retValue;
    int imgType = ImageType::IMAGE_RGB_FULL_WITH_TEXT;
    retValue += "<table border=\"1\">";
    for (size_t i = 0; i < CImageBuffer::getBufferInstance().getBuffersSize(); i++)
    {

        if (CImageBuffer::getBufferInstance().getHasAlert(i))
        {
            retValue += "<tr bgcolor=red>";
        }
        else
        {
            retValue += "<tr>";
        }
        retValue += "<td><a  href=/" + QString::number(i) + "/" + QString::number(imgType) + "/1"
                + "&user=" + QString::fromStdString(mConfig.mUserName)
                + "&password="  + QString::fromStdString(mConfig.mPassword) + "/>"
                + QString::number(i) + "  " + QString::fromStdString(CImageBuffer::getBufferInstance().getBufferInfo(i)) + "    "
                + "</td>"
                + "<td>HasMotion:" + QString::number(CImageBuffer::getBufferInstance().getHasAlert(i)) + "</td>"
                + " <td>size " + QString::number(CImageBuffer::getBufferInstance().size(i))
                + " </td><td>(long buffer size " + QString::number(CImageBuffer::getBufferInstance().longSize(i)) + ")"
                +  " </td><td>is Full " + QString::number(static_cast<int>(CImageBuffer::getBufferInstance().isFull(i)))
                + "</td>";

        if (!CImageBuffer::getBufferInstance().empty(i))
        {
            retValue += " <td>   " + QString::fromStdString(CImageBuffer::getBufferInstance().getLast(i)->getDateTimeWithMS());
            retValue += " </td><td> last id: " + QString::number(CImageBuffer::getBufferInstance().getLast(i)->getId());
            retValue += "</td>";
            if (!CImageBuffer::getBufferInstance().getLongList(i).empty())
            {
                retValue += " <td>Long DT " + QString::fromStdString(CImageBuffer::getBufferInstance().getLongList(i).last()->getDateTimeWithMS()) + "</td>";
            }
        }

        if (CImageBuffer::getBufferInstance().getLastAlertDt(i).isValid())
        {
           retValue += "<td>Last alert " + CImageBuffer::getBufferInstance().getLastAlertDt(i).toString(sDateTimeWithMSToStrFormat) + "</td>";
        }
        retValue += + "</tr>";
    }
    retValue += + "</table>";
    return retValue;
}

void CClientConnectionMJPEG::slotError(QAbstractSocket::SocketError error)
{
    LOG_ERROR << "error:" << error << mSocket->errorString();
}

void CClientConnectionMJPEG::timerTimeout()
{
    writeImage();
}
}
