#include "CSerializer.hpp"
#include "Image/CImageBuffer.hpp"
#include "ImageServer/CImageSenderWorker.hpp"
#include "logging/Log.hpp"

NNet::pWorker CImageSenderWorker::create()
{
    return std::shared_ptr<CImageSenderWorker>(new CImageSenderWorker());
}

CImageSenderWorker::CImageSenderWorker()
{

}

CImageSenderWorker::~CImageSenderWorker()
{
}

void CImageSenderWorker::receiveCallBack(const QByteArray& receiveStr)
{
    static_assert(static_cast<int>(NNet::CNetProtocol::ANSVER::MAX_VALUE) == 11, "change in ANSVER struct");
    //TODO: parse receiveStr and send once
    size_t bufferId;
    std::string errorStr;
    QString clientRequestStr2;
    NNet::CNetProtocol::ANSVER ansver;
    int imageType;
    NTypes::tIdType imageQuality;
    NNet::tAllowedUsers users; //TODO: tmp, move to config

    if (!CSerializer::load(ansver, clientRequestStr2, imageType, imageQuality, receiveStr, users, errorStr))
    {
        LOG_DEBUG << "fail decerialize client ansver" << errorStr;
        //todo: return
    }
    auto result = parse(ansver, clientRequestStr2, bufferId);

    QByteArray str;
    if (result == NNet::CNetProtocol::ANSVER::IMAGE_ID)
    {
        if (imageType >= 0 && imageType < ImageType::MAX_VALUE)
        {
            auto type = static_cast<ImageType::EType>(imageType);
            CSerializer::save(
                        type,
                        bufferId,
                        NNet::CNetProtocol::ANSVER::IMAGE_ID,
                        imageQuality,
                        str);
            mpNet->send(str);
        }
        else
        {
            CSerializer::save(
                        ImageType::MAX_VALUE,
                        0,
                        NNet::CNetProtocol::ANSVER::IMAGE_TYPE_OUT_OF_RANGE,
                        imageQuality,
                        str,
                        "request for image type " + QString::number(imageType));
            mpNet->send(str);
        }
    }
    else if (result == NNet::CNetProtocol::ANSVER::BUFFER_INDEX_OUT_OF_RANGE)
    {
        CSerializer::save(
                    ImageType::MAX_VALUE,
                    0,
                    NNet::CNetProtocol::ANSVER::BUFFER_INDEX_OUT_OF_RANGE,
                    imageQuality,
                    str,
                    "request for buffer " + QString::number(bufferId)
                    + " but size is "
                    + QString::number(CImageBuffer::getBufferInstance().getBuffersSize()));
        mpNet->send(str);
    }
    else if (result == NNet::CNetProtocol::ANSVER::TEXT_BUFFER)
    {
        CSerializer::save(
                    ImageType::MAX_VALUE,
                    0,
                    NNet::CNetProtocol::ANSVER::TEXT_BUFFER,
                    imageQuality,
                    str,
                    "request for buffer " + QString::number(bufferId)
                    + " but size is "
                    + QString::number(CImageBuffer::getBufferInstance().getBuffersSize()));
        mpNet->send(str);
    }
    else
    {
        //TODO: implement this
        LOG_DEBUG << "else, not implemented" << static_cast<int>(ansver) << clientRequestStr2;

        CSerializer::save(
                    ImageType::MAX_VALUE,
                    0,
                    NNet::CNetProtocol::ANSVER::NOT_IMPLEMENTED,
                    imageQuality,
                    str);
        mpNet->send(str);
    }
}

void CImageSenderWorker::sendCallBack(qint64)
{

}

void CImageSenderWorker::onError(
        QAbstractSocket::SocketError sockError,
        const QString& errorDescription,
        const QString& clientDescription)
{
    LOG_DEBUG << "error: " << static_cast<int>(sockError) << errorDescription << clientDescription;
}

NNet::CNetProtocol::ANSVER CImageSenderWorker::parse(
        NNet::CNetProtocol::ANSVER ansver,
        const QString& str2,
        size_t& bufferId)
{
    if (ansver == NNet::CNetProtocol::ANSVER::IMAGE_ID)
    {
        //bufferId = 0;
        bool ok;
        bufferId = str2.toUInt(&ok);
        if (ok)
        {
            if (bufferId < CImageBuffer::getBufferInstance().getBuffersSize())
            {
                return NNet::CNetProtocol::ANSVER::IMAGE_ID;
            }
            else
            {
                LOG_DEBUG << "buffer out of range " << bufferId
                          << "max size" << CImageBuffer::getBufferInstance().getBuffersSize();
                return NNet::CNetProtocol::ANSVER::BUFFER_INDEX_OUT_OF_RANGE;
            }
        }
        else
        {
            return NNet::CNetProtocol::ANSVER::CODE_PARSE_FAIL;
        }
    }
    return ansver;
}

void CImageSenderWorker::onConnect()
{

}

