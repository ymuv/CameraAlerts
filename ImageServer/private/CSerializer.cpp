#include <QDataStream>
#include <QDateTime>

#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <mutex>

#include "CSerializer.hpp"
#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "Net/CNetValues.hpp"
#include "Text/CTextBuffer.hpp"
#include "common/CMemoryUssage.hpp"
#include "common/CConnectionSettings.hpp"
#include "common/CHashFunctions.hpp"
#include "logging/Log.hpp"


using NNet::CNetProtocol;
using NNet::CNetValues;

void CSerializer::save(
        const std::shared_ptr<CMatWithTimeStamp> mat,
        ImageType::EType type,
        CNetProtocol::ANSVER ansver,
        NTypes::tIdType imageQuality,
        QByteArray& outArray,
        const QString& infoStr)
{
    outArray.clear();
    static_assert(ImageType::MAX_VALUE == 6, "Change ImageType enum");
    {
        QDataStream stream(&outArray, QIODevice::ReadWrite);

        stream.setVersion(CNetValues::sDataStreamVersion);
        stream << CNetValues::sVersion;
        stream << infoStr;
        stream << static_cast<NTypes::tAnsver>(ansver);  //TODO: in server (actual??)

        if (ansver == CNetProtocol::ANSVER::IMAGE_ID)
        {
            std::vector<int> compressionParams;
            compressionParams.push_back(CV_IMWRITE_JPEG_QUALITY);
            compressionParams.push_back(imageQuality);

            std::vector<uchar> buff;
            cv::imencode(".jpg", mat->getMat(type), buff, compressionParams);
            NTypes::tBuffSizeTmp buffSize = buff.size(); //fix fail compile on gcc-4.9
            const char* charArray =  reinterpret_cast<const char*>(&buff.at(0));

            stream <<  mat->getId();
            stream << mat->getDateTimeCreation();
            stream << buffSize;

            stream.writeRawData(charArray, buff.size());
            stream << CMemoryUssage::processMemoryUssage();

            CHashFunctions::writeHash(outArray, stream);
        }
        else if (ansver == CNetProtocol::ANSVER::TEXT_BUFFER)
        {
            stream <<  NText::CTextBuffer::getInstance().getMap();
            CHashFunctions::writeHash(outArray, stream);
        }
        else
        {
            CHashFunctions::writeHash(outArray, stream);
        }
    }
}

void CSerializer::save(
        ImageType::EType type,
        size_t bufferId,
        CNetProtocol::ANSVER ansver,
        NTypes::tIdType imageQuality,
        QByteArray& array,
        const QString& infoStr)
{
    if (CImageBuffer::getBufferInstance().getBuffersSize() <= bufferId)
    {
        save(nullptr, type, CNetProtocol::ANSVER::BUFFER_INDEX_OUT_OF_RANGE, imageQuality, array, infoStr);
    }
    CImageBuffer buff = CImageBuffer::getBufferInstance();
    pMatWithTimeStamp mat = buff.getLast(bufferId);

    if (mat)
    {
        CMatWithTimeStampLocker locker(mat);
        save(mat, type, ansver, imageQuality, array, infoStr);
    }
    else
    {
        save(nullptr, type, CNetProtocol::ANSVER::MAT_NULL_PTR, imageQuality, array,
                    "image is empty, maybe fail open device; id" + QString::number(bufferId));
    }
}

bool CSerializer::load(
        CNetProtocol::ANSVER& ansver,
        QString& str2,
        int& imageType,
        NTypes::tIdType& ansverData,
        const QByteArray& inputStr,
        const NNet::tAllowedUsers& allowedUsers,
        std::string& errorString)
{
    QDataStream stream(inputStr);

    stream.setVersion(CNetValues::sDataStreamVersion);

    NTypes::tVersionType version;
    NTypes::tAnsver ansverInt;
    NTypes::tHash hash;
    NTypes::tHashPosition hashPosition;
    QString user, password;

    stream >> version;
    stream >> user;
    stream >> password;

    stream >> imageType;
    stream >> ansverInt;
    stream >> ansverData;
    stream >> str2;
    stream >> hashPosition;
    stream >> hash;

    if (version != CNetValues::sVersion)
    {
        LOG_DEBUG << "invalid version";
        errorString = "fail version, current: " + std::to_string(CNetValues::sVersion)
                + "; getted: " + std::to_string(version);
        return false;
    }

    if (!CHashFunctions::checkHash(inputStr, hash, hashPosition))
    {
        errorString = "fail hash" + std::to_string(hashPosition);
        return false;
    }
    ansver = static_cast<NNet::CNetProtocol::ANSVER>(ansverInt);
    return true;
}
