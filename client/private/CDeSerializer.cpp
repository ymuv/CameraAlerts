#include <QDataStream>
#include <opencv2/highgui/highgui.hpp>

#include "CDeSerializer.hpp"
#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "Net/CNetValues.hpp"
#include "Text/CTextBuffer.hpp"
#include "common/CHashFunctions.hpp"
#include "logging/Log.hpp"

using NNet::CNetProtocol;
using NNet::CNetValues;

//refactor, use enum
bool CDeSerializer::load(
        CMatWithTimeStamp& mat,
        const QByteArray& inputStr,
        std::string& errorString,
        size_t bufferId,
        size_t lastId)
{
    static_assert(static_cast<int>(CNetProtocol::ANSVER::MAX_VALUE) == 11, "change in ANSVER struct");
    QDataStream stream(inputStr);

    stream.setVersion(CNetValues::sDataStreamVersion);

    NTypes::tVersionType version;
    QString infoStr;
    NTypes::tAnsver ansverId;

    stream >> version;
    stream >> infoStr;
    stream >> ansverId;

    if (version != CNetValues::sVersion)
    {
        LOG_DEBUG << "invalid version";
        errorString = "fail version, current: " + std::to_string(CNetValues::sVersion)
                + "; getted: " + std::to_string(version);
        return false;
    }

    CNetProtocol::ANSVER ansver = static_cast<NNet::CNetProtocol::ANSVER>(ansverId);

    if (ansver ==CNetProtocol::ANSVER::IMAGE_ID)
    {
       NTypes::tMatIdType id;
       stream >> id;

       if (lastId == id)
       {
          errorString = "repeat number " + std::to_string(id);
          LOG_DEBUG << errorString << bufferId;
          return false;
       }
       QDateTime dt;
       NTypes::tBuffSizeTmp buffSizeTmp;
       NTypes::tMemUssage memoryUssage;
       NTypes::tHash hash;
       NTypes::tHashPosition hashPosition;

       stream >> dt;
       stream >> buffSizeTmp;

       if (buffSizeTmp <= 0 || inputStr.size() < buffSizeTmp)
       {
          errorString = "wrong buffer size; get:"
                + std::to_string(buffSizeTmp) + " receive str size:"
                + std::to_string(inputStr.size());
          return false;
       }

       std::vector<uchar>buff(buffSizeTmp);
       char* charArray =  reinterpret_cast< char*>(&buff.at(0));
       stream.readRawData(charArray, buffSizeTmp);

       stream >> memoryUssage;

       stream >> hashPosition;
       stream >> hash;

       if (!CHashFunctions::checkHash(inputStr, hash, hashPosition))
       {
          errorString = "fail hash "
                  + std::to_string(hash) + " "
                  + std::to_string(hashPosition) + " "
                  + std::to_string(inputStr.size());
          return false;
       }

       auto m = cv::imdecode(buff, 1);
       mat.copyToThis(m);
       mat.setDateTimeCreation(dt);
       mat.setID(id);
       return true;
    }
    else if (ansver == CNetProtocol::ANSVER::BUFFER_EMPTY
             || ansver == CNetProtocol::ANSVER::BUFFER_INDEX_OUT_OF_RANGE)
    {
       LOG_WARN << "empty" << ansverId << static_cast<int >(ansver) << infoStr;
       return false;
    }
    else if (ansver == CNetProtocol::ANSVER::MAT_NULL_PTR)
    {
        LOG_WARN << "last image empty"<< infoStr;
        return false;
    }
    else if (ansver == CNetProtocol::ANSVER::TEXT_BUFFER)
    {
        NText::CTextBuffer::tMapType map;
        stream >> map;
        for (const auto& key : map.keys())
        {
            NText::CTextBuffer::getInstance().setList(key, map[key]);
        }

        return true;
    }
    else if (ansver == CNetProtocol::ANSVER::NOT_IMPLEMENTED)
    {
        LOG_WARN << "not implemented"<< infoStr;
        return false;
    }
    else
    {
       LOG_WARN << "not implemented; ansver id: " << (int)ansver << infoStr;
       return false;
    }
}


void CDeSerializer::ClientSerialize(
        const CNetProtocol::ANSVER ansver,
        const ImageType::EType type,
        const QString& str,
        const NTypes::tIdType ansverData,
        const QString& user,
        const QString& pass,
        QByteArray& outStr)
{
    static_assert(ImageType::MAX_VALUE == 6, "Change ImageType enum change");
    QDataStream stream(&outStr, QIODevice::ReadWrite);
    int imageType = type;

    stream.setVersion(CNetValues::sDataStreamVersion);
    stream << CNetValues::sVersion;
    stream << user;
    stream << pass;

    stream << imageType;
    stream << static_cast<NTypes::tAnsver>(ansver);
    stream << ansverData;
    stream << str;
    CHashFunctions::writeHash(outStr, stream);
}

