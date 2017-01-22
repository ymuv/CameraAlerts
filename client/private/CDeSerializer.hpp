#pragma once
#include <QByteArray>

#include "Image/ImageType.hpp"
#include "common/CNetProtocol.hpp"

class CMatWithTimeStamp;

class CDeSerializer
{
public:
    //const std::string& CSerializer::save()
    static bool load(
            CMatWithTimeStamp&,
            const QByteArray& inputStr,
            std::string& errorString,
            size_t bufferId,
            size_t lastId);

    //parse with bool CSerializer::load()
    static void ClientSerialize(
            const NNet::CNetProtocol::ANSVER ansver,
            const ImageType::EType type,
            const QString& str,
            const NTypes::tIdType ansverData,
            const QString& user,
            const QString& pass,
            QByteArray& outStr);
};
