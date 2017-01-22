#pragma once

#include <string>
#include <memory>
#include <QByteArray>

#include "Image/ImageType.hpp"
#include "Net/CUserData.hpp"
#include "common/CNetProtocol.hpp"


class CMatWithTimeStamp;

class CSerializer
{
public: //static methods

    static void save(
            const std::shared_ptr<CMatWithTimeStamp> mat,
            ImageType::EType type,
            NNet::CNetProtocol::ANSVER ansver,
            NTypes::tIdType imageQuality,
            QByteArray& array,
            const QString& infoStr);

    static void save(
            ImageType::EType type,
            size_t bufferId,
            NNet::CNetProtocol::ANSVER ansver,
            NTypes::tIdType imageQuality,
            QByteArray& array,
            const QString& infoStr = "");

    //void CDeSerializer::ClientSerialize - generator
    static bool load(
            NNet::CNetProtocol::ANSVER& ansver,
            QString& str2,
            int& imageType,
            NTypes::tIdType& ansverData,
            const QByteArray& inputStr,
            const NNet::tAllowedUsers& allowedUsers,
            std::string& errorString);
};

