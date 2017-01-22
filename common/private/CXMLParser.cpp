#include <QFile>
#include <QXmlStreamReader>

#include <db/CDBConfig.hpp>

#include "webDownload/CWebDownloaderConfig.hpp"

#include "conditions/CCondition.hpp"

#include "alerts/CCommonAlertConfig.hpp"

#include "MJPEGServer/CMJPEGServerConfig.hpp"
#include "Image/motionAlgorithms/CFaceDetectConfig.hpp"
#include "Image/motionAlgorithms/CMoveDetectConfig.hpp"
#include "Image/motionAlgorithms/CMoveDetectBackgroundSubtractorConfig.hpp"
#include "Image/motionAlgorithms/CPHashConfig.hpp"
#include "Image/videoInput/CInputConfig.hpp"
#include "Net/CNetCryptionConfig.hpp"
#include "Text/CTextPatterns.hpp"
#include "alerts/CAlertSoundConfig.hpp"
#include "alerts/CAlertConfig.hpp"
#include "alerts/CFinalVideoWriterConfig.hpp"
#include "alerts/CTelegramAlertNotificationConfig.hpp"
#include "alerts/CSMTPAlertConfig.hpp"
#include "common/CAlgoConfig.hpp"
#include "common/CConfig.hpp"
#include "common/CConnectionSettings.hpp"
#include "common/private/CXMLParser.hpp"
#include "command/CCommand.hpp"
#include "logging/Log.hpp"
#include "serial/CSerialPortConfig.hpp"

namespace
{
const QString patternListsStr = "Patterns";
const QString patternStr = "Pattern";
const QString commonAlertStr = "CommonAlert";
const QString commandsStr = "Commands";
const QString inputStr = "Input";


#define PARSE_IF_NOT_EXIST(attributes, xmlName, configName, toType) \
    if (attributes.hasAttribute(xmlName)) { \
        configName = attributes.value(xmlName).toType(); \
        LOG_DEBUG <<  " parse " << xmlName << attributes.value(xmlName) << xmlName << "="<< configName; \
}
#define PARSE_IF_NOT_EXIST2(xmlName, toType) \
    PARSE_IF_NOT_EXIST(attributes, #xmlName, config.m##xmlName, toType)

#define PARSE(attributes, xmlName, configName, defaultValue, toType) \
    if (attributes.hasAttribute(xmlName)) { \
        configName = attributes.value(xmlName).toType(); \
        LOG_DEBUG <<  " parse " << xmlName << attributes.value(xmlName) << xmlName << "="<< configName; \
} \
    else { \
        LOG_WARN << " using default for " << xmlName << defaultValue; \
        configName = defaultValue;\
}

//TODO: from PARSE

#define PARSE0(attributes, xmlName, configName, defaultValue, toType, config) \
    if (attributes.hasAttribute(xmlName)) { \
        config.configName = attributes.value(xmlName).toType(); \
        LOG_DEBUG <<  " parse " << xmlName << attributes.value(xmlName) << xmlName << "="<< config.configName; \
} \
    else { \
        LOG_WARN << " using default for " << xmlName << defaultValue; \
        config.configName = defaultValue;\
}

#define PARSE1(attributes, xmlName, configName, defaultValue, toType) \
    PARSE0(attributes, xmlName, configName, defaultValue, toType, config)

#define PARSE2(xmlName, defaultValue, toType) \
    PARSE1(attributes, #xmlName, m##xmlName, defaultValue, toType)

void resizeToSize(std::string& inStr, size_t sizeToResize)
{
    std::string tempStr(sizeToResize, 0);
    for (size_t i = 0, j =0; i < sizeToResize; i++)
    {
        tempStr[i] =  inStr[j];
        j++;
        if (j >= inStr.size())
        {
            j = 0;
        }
    }
    inStr = tempStr;
}

template <class T>
void parseArray(
        QXmlStreamAttributes attributes,
        const QString& valueToParse,
        std::vector<T>& vectorToAdd)
{
    QStringList qlist;
    {
        std::string arrayStr;
        PARSE(attributes, valueToParse, arrayStr, "", toString().toStdString);
        qlist = QString::fromStdString(arrayStr).
                split(";", QString::SkipEmptyParts);
        foreach(QString str, qlist)
        {
            str = str.replace("\n", "").replace(" ","");
            T value = QVariant(str).value<T>();
            vectorToAdd.push_back(value);
        }
    }
}

void parseBaseAlert(NAlarms::CAlertConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(ForceSendInterval, 3300, toInt);
    PARSE2(MinIntervalRepeatSend, 20, toInt);
    PARSE2(ReplyDiffTime, 20, toInt);
    PARSE2(FirstTimeSkip, 20, toInt);
    PARSE2(ReplyCount, 3, toInt);
}

void parseIfNotExist(NAlarms::CAlertConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE_IF_NOT_EXIST2(ForceSendInterval, toInt);
    PARSE_IF_NOT_EXIST2(MinIntervalRepeatSend, toInt);
    PARSE_IF_NOT_EXIST2(ReplyDiffTime, toInt);
    PARSE_IF_NOT_EXIST2(FirstTimeSkip, toInt);
    PARSE_IF_NOT_EXIST2(ReplyCount, toInt);
}

void parseCondtion(NCondition::CConditionList& configList, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    NCondition::CCondition config;
    PARSE2(Expression, "", toString);
    PARSE2(Alert, "", toString);
    PARSE2(Command, "", toString);

    if (!config.mExpression.isEmpty())
    {
        configList.mConditions.push_back(config);
    }
}

void parseCommand(NCommand::CCommand& config, const QXmlStreamReader& xml, QString& name)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE(attributes, "Name", name, "", toString);
    PARSE2(Url, "", toString);
    PARSE2(SerialPort, "", toString);
    PARSE2(SerialCommand, "", toString);
    PARSE2(Say, "", toString);
}

void parseCommands(NCommand::CCommandsMap& commandsMap, QXmlStreamReader& xml)
{
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == commandsStr))
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "Command")
            {
                NCommand::CCommand command;
                QString name;
                parseCommand(command, xml, name);
                if (!name.isEmpty()
                        && (!command.mSay.isEmpty()
                            || !command.mUrl.isEmpty()
                            || (!command.mSerialPort.isEmpty() && !command.mSerialCommand.isEmpty())))
                {
                    commandsMap.mCommandMap[name] = command;
                }
            }
        }
        if (token == QXmlStreamReader::Invalid)
        {
            LOG_ERROR << "error xml " << xml.errorString();
            throw "error xml " + xml.errorString().toStdString(); //TODO:
        }
    }
}


void parseSound(NAlarms::CAlertSoundConfig& config,const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(Sound, "", toString);
}

void parseTelegram(
        NAlarms::CTelegramAlertNotificationConfig& config,
        const QXmlStreamReader& xml,
        const NAlarms::CAlertConfig& baseConfig)
{
    NAlarms::CAlertConfig& alertConf = config;
    alertConf = baseConfig;

    parseIfNotExist(static_cast<NAlarms::CAlertConfig&>(config), xml);
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(Token, "", toString().toStdString);
    PARSE2(IsSendImagesWhenMaskSendet, false, toInt);
    parseArray<int64_t>(attributes, "VerifyUsers", config.mVerifyUsers);
    parseArray<int64_t>(attributes, "AlertUsers", config.mAlertUsers);
}

void parseFinalVideo(NAlarms::CFinalVideoWriterConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(IsWriteMaskToFinalVideo, true, toInt);
    PARSE2(FrameBeforeAlertToVideo, 3, toInt);
    PARSE2(FrameAfterAlertToVideo, 5, toInt);
    PARSE2(VideoFps, 8.0, toDouble);
    QStringList qlist;
    //parse files
    {
        std::string files;
        PARSE(attributes, "SaveFinalVideoPaths", files,
              "", toString().toStdString);
        qlist = QString::fromStdString(files).split(";", QString::SkipEmptyParts);

        foreach( QString str, qlist)
        {
            str = str.replace("\n", "").replace(" ","");
            if (!str.isEmpty())
            {
                config.mSaveFinalVideoPaths.push_back(str.toStdString());
            }
        }
    }
}

void parseSMTPConfig(NAlarms::CSMTPAlertConfig& config, const QXmlStreamReader& xml,const NAlarms::CAlertConfig& baseConfig);

void parseCommonAlert(
        NAlarms::CCommonAlertConfig& config,
        const  NAlarms::CAlertConfig& baseAlertConfig,
        QXmlStreamReader& xml)
{
    using namespace NAlarms;
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == commonAlertStr))
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "Telegram")
            {
                if (!config.mTelegramConfig)
                {
                    config.mTelegramConfig.reset(new CTelegramAlertNotificationConfig);
                }
                parseTelegram(*config.mTelegramConfig, xml, baseAlertConfig);
            }
            else if (xml.name() == "AlertSound")
            {
                if (!config.mAlertSoundConfig)
                {
                    config.mAlertSoundConfig.reset(new CAlertSoundConfig);
                }
                parseSound(*config.mAlertSoundConfig, xml);
            }
            else if (xml.name() == "FinalVideo")
            {
                if (!config.mFinalVideoConfig)
                {
                    config.mFinalVideoConfig.reset(new CFinalVideoWriterConfig);
                }
                parseFinalVideo(*config.mFinalVideoConfig, xml);
            }
            else if (xml.name() == "smtp")
            {
                if (!config.mSMTPConfig)
                {
                    config.mSMTPConfig.reset(new CSMTPAlertConfig);
                }
                parseSMTPConfig(*config.mSMTPConfig, xml, baseAlertConfig);
            }
        }
        if (token == QXmlStreamReader::Invalid)
        {
            LOG_ERROR << "error xml " << xml.errorString();
            throw "error xml " + xml.errorString().toStdString(); //TODO:
        }
    }
}

void parse(NMJPEG::CMJPEGServerConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(Port, 8081, toUShort);
    PARSE2(JPEGQuality, 90, toInt);
    PARSE2(RefreshTimeMS, 100, toInt);

    PARSE2(UserName, "", toString().toStdString);
    PARSE2(Password, "", toString().toStdString);
}

void parse(NNet::CNetCryptionConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(Key, "Key1", toString().toStdString);
    PARSE2(KeySize, 16, toUInt);

    PARSE2(Key2, "Key2", toString().toStdString);
    PARSE2(Key2Size, 16, toUInt);

    PARSE2(Key3, "Key2", toString().toStdString);
    PARSE2(Key3Size, 16, toUInt);

    PARSE2(InitialVector, "Vector", toString().toStdString);
    PARSE2(IsUseCryption, false, toUInt);

    //resize keys and initVector. TODO: do simply
    if  (!config.mKey.empty())
    {
        resizeToSize(config.mKey, config.mKeySize);
    }
    if  (!config.mKey2.empty())
    {
        resizeToSize(config.mKey2, config.mKey2Size);
    }
    if  (!config.mKey3.empty())
    {
        resizeToSize(config.mKey3, config.mKey3Size);
    }
    resizeToSize(config.mInitialVector, config.sBlockSize);
}

void parseInputAlgoCoef(NVideoInput::CAlgoCoeficients& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(AlgoCoenficient, 1, toDouble);
    PARSE2(AlgoTrashCoenficient, 1, toDouble);
    PARSE2(IsRunMotionAlgo, true, toUInt);
}

void parseInputBase(NVideoInput::CInputConfigEntry& config, const QXmlStreamReader& xml)
{
    parseInputAlgoCoef(config.mAlgoCoeficient, xml);
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(ResizeScaleFactor, 1, toDouble);
    PARSE2(ResizeScaleFactorForNet, 2, toDouble);

    PARSE2(SleepTime, 1, toDouble);
    PARSE2(BufferSize, 5, toUInt);
    PARSE2(LongBufferSize, 10, toInt);
    PARSE2(LongBufferSkip, 10, toInt);

    PARSE2(TimeOut, 4, toInt);

    PARSE2(FrameWidth, 1280, toInt);
    PARSE2(FrameHeight, 1024, toInt);
}

bool parseInputURLList(
        std::vector<NVideoInput::CInputConfigEntry>& configs,
        const NVideoInput::CInputConfigEntry& baseConfig,
        const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    NVideoInput::CInputConfigEntry config = baseConfig;
    PARSE_IF_NOT_EXIST(attributes, "AlgoCoenficient", config.mAlgoCoeficient.mAlgoCoenficient, toDouble);
    PARSE_IF_NOT_EXIST(attributes, "AlgoTrashCoenficient", config.mAlgoCoeficient.mAlgoTrashCoenficient, toDouble);
    PARSE_IF_NOT_EXIST(attributes, "IsRunMotionAlgo", config.mAlgoCoeficient.mIsRunMotionAlgo, toUInt);

    PARSE_IF_NOT_EXIST2(ResizeScaleFactor, toDouble);
    PARSE_IF_NOT_EXIST2(ResizeScaleFactorForNet, toDouble);
    PARSE_IF_NOT_EXIST2(SleepTime, toDouble);
    PARSE_IF_NOT_EXIST2(BufferSize, toUInt);
    PARSE_IF_NOT_EXIST2(LongBufferSize, toInt);
    PARSE_IF_NOT_EXIST2(LongBufferSkip, toInt);

    config.mType = NVideoInput::InputType::URL;

    PARSE_IF_NOT_EXIST2(TimeOut, toInt);


    QString connections;
    PARSE(attributes, "Connection", connections, "", toString);
    if (connections.isEmpty())
    {
        return false;
    }

    auto qlist = connections.split(";", QString::SkipEmptyParts);
    foreach(QString str, qlist)
    {
        str = str.replace("\n", "").replace(" ","");
        int colonPos = str.lastIndexOf(':');

        if(colonPos != -1)
        {
            NNet::CNetConnectionConfig connection;
            PARSE0(attributes, "CompressionLevel", mCompressionLevel, 55, toUInt, connection);
            auto portAndImageId = str.mid(colonPos+1);
            int pos = portAndImageId.lastIndexOf('/');

            if (pos > 0)
            {
                connection.mPort = portAndImageId.mid(0, pos).toUShort();
                bool ok;
                if (portAndImageId.mid(pos + 1) == "TextBuffer")
                {
                    connection.mIsTextBuffer = true;
                    connection.mRequestdImageId = static_cast<ImageType::EType>(ImageType::MAX_VALUE);
                    connection.mImageId = 0;
                }
                else
                {
                    int posAtSymbol = portAndImageId.lastIndexOf('@');
                    connection.mIsTextBuffer = false;
                    connection.mImageId = portAndImageId.mid(pos + 1, posAtSymbol - pos - 1).toUShort(&ok);
                    int imageType = portAndImageId.mid(posAtSymbol + 1).toInt();
                    if (imageType < 0 || imageType >= ImageType::MAX_VALUE)
                    {
                        LOG_WARN << "fail image type" << imageType;
                    }
                    connection.mRequestdImageId = static_cast<ImageType::EType>(imageType);
                }
            }
            else
            {
                connection.mPort = portAndImageId.toUShort();
            }

            auto hostAndProtocol = str.mid(0,colonPos);

            pos = hostAndProtocol.lastIndexOf(':');
            auto protocol = hostAndProtocol.mid(0, pos);

            pos = hostAndProtocol.lastIndexOf('/');

            connection.mServerHost = hostAndProtocol.mid(pos + 1).toStdString();

            if (protocol == "TCP" || protocol == "UDP" || protocol == "TCP_SSL")
            {
                static_assert(static_cast<int>(NNet::TypeProtocol::MAX_VALUE) == 3, "max value != 3, fix it");
                connection.mTypeProtocol = NNet::TypeProtocol::TCP;
                if (protocol == "TCP")
                {
                    connection.mTypeProtocol = NNet::TypeProtocol::TCP;
                }
                else if (protocol == "UDP")
                {
                    connection.mTypeProtocol = NNet::TypeProtocol::UDP;
                }
                else if (protocol == "TCP_SSL")
                {
                    connection.mTypeProtocol = NNet::TypeProtocol::TCP_SSL;
                }
                NVideoInput::CInputConfigEntry copy = config;
                copy.mConnectionConfig = connection;
                configs.push_back(copy);
            }
            else
            {
                LOG_FATAL << "bad protocol type" << protocol;
            }
        }
    }
    return true;
}

bool parseInputConfigEntry(NVideoInput::CInputConfigEntry& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE_IF_NOT_EXIST(attributes, "AlgoCoenficient", config.mAlgoCoeficient.mAlgoCoenficient, toDouble);
    PARSE_IF_NOT_EXIST(attributes, "AlgoTrashCoenficient", config.mAlgoCoeficient.mAlgoTrashCoenficient, toDouble);
    PARSE_IF_NOT_EXIST(attributes, "IsRunMotionAlgo", config.mAlgoCoeficient.mIsRunMotionAlgo, toUInt);

    PARSE_IF_NOT_EXIST2(ResizeScaleFactor, toDouble);
    PARSE_IF_NOT_EXIST2(ResizeScaleFactorForNet, toDouble);
    PARSE_IF_NOT_EXIST2(SleepTime, toDouble);
    PARSE_IF_NOT_EXIST2(BufferSize, toUInt);
    PARSE_IF_NOT_EXIST2(LongBufferSize, toInt);
    PARSE_IF_NOT_EXIST2(LongBufferSkip, toInt);

    if (config.mType == NVideoInput::InputType::URL)
    {
        PARSE_IF_NOT_EXIST2(TimeOut, toInt);

        //parse connections
        {
            std::string connection;
            PARSE(attributes, "Connection", connection, "", toString().toStdString);
            if (connection.empty())
            {
                return false;
            }

            QString str = QString::fromStdString(connection);
            {
                str = str.replace("\n", "").replace(" ","");
                int colonPos = str.lastIndexOf(':');

                if(colonPos != -1)
                {
                    NNet::CNetConnectionConfig connection;
                    auto portAndImageId = str.mid(colonPos+1);
                    int pos = portAndImageId.lastIndexOf('/');
                    int posAtSymbol = portAndImageId.lastIndexOf('@');
                    if (pos > 0)
                    {
                        connection.mPort = portAndImageId.mid(0, pos).toUShort();
                        connection.mImageId = portAndImageId.mid(pos + 1, posAtSymbol - pos - 1).toUShort();
                        int imageType = portAndImageId.mid(posAtSymbol + 1).toInt();
                        if (imageType < 0 || imageType >= ImageType::MAX_VALUE)
                        {
                            LOG_WARN << "fail image type" << imageType;
                        }
                        connection.mRequestdImageId = static_cast<ImageType::EType>(imageType);
                    }
                    else
                    {
                        connection.mPort = portAndImageId.toUShort();
                    }

                    auto hostAndProtocol = str.mid(0,colonPos);

                    pos = hostAndProtocol.lastIndexOf(':');
                    auto protocol = hostAndProtocol.mid(0, pos);

                    pos = hostAndProtocol.lastIndexOf('/');

                    connection.mServerHost = hostAndProtocol.mid(pos + 1).toStdString();

                    if (protocol == "TCP" || protocol == "UDP")
                    {
                        connection.mTypeProtocol = NNet::TypeProtocol::TCP;
                        if (protocol == "TCP")
                        {
                            connection.mTypeProtocol = NNet::TypeProtocol::TCP;
                        }
                        else if (protocol == "UDP")
                        {
                            connection.mTypeProtocol = NNet::TypeProtocol::UDP;
                        }
                        else
                        {
                            LOG_ERROR << "unknwown protocol type in config";
                        }
                        config.mConnectionConfig = connection;
                    }
                    else
                    {
                        LOG_FATAL << "bad protocol type" << protocol;
                    }
                }
            }
        }

    }
    else if (config.mType == NVideoInput::InputType::FILE)
    {
        PARSE2(File, "", toString().toStdString);
    }
    else if (config.mType == NVideoInput::InputType::CAMERA)
    {
        PARSE2(CameraId, 1, toInt);
        PARSE_IF_NOT_EXIST2(FrameWidth, toInt);
        PARSE_IF_NOT_EXIST2(FrameHeight, toInt);
    }
    return true;
}

void parse(CAlgoBaseConfig& config, const QXmlStreamAttributes& attributes)
{
    PARSE2(IsRun, false, toInt);
    PARSE2(SkipFrames, 1, toUInt);
}

void parse(NAlgorithms::CPHashConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    parse((CAlgoBaseConfig&) config, attributes);
    PARSE2(MinDistance,  10, toInt);
}

void parseIfNotExist(CAlgoBaseConfig& config, const QXmlStreamAttributes& attributes)
{
    PARSE_IF_NOT_EXIST2(IsRun, toInt);
    PARSE_IF_NOT_EXIST2(SkipFrames, toUInt);
}

void parseBase(CMoveDetectConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    parse((CAlgoBaseConfig&) config, attributes);
    PARSE2(AreaMinSize,  800, toUInt);
    PARSE2(MinTrashHold, 35, toInt);
    PARSE2(IsUseErode, true, toInt);
    PARSE2(IsUseDilate, true, toInt);

    PARSE2(MinRectangeWidth, 10, toInt);
    PARSE2(MinRectangeHeight, 10, toInt);

    PARSE2(GaussianBlurIndexBegin,  0, toUInt);
    PARSE2(GaussianBlurIndexEnd, 0, toUInt);
}

void parse(CMoveDetectConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    parseIfNotExist((CAlgoBaseConfig&) config, attributes);
    PARSE_IF_NOT_EXIST2(AreaMinSize, toUInt);
    PARSE_IF_NOT_EXIST2(MinTrashHold, toInt);
    PARSE_IF_NOT_EXIST2(IsUseErode, toInt);
    PARSE_IF_NOT_EXIST2(IsUseDilate, toInt);

    PARSE_IF_NOT_EXIST2(MinRectangeWidth, toInt);
    PARSE_IF_NOT_EXIST2(MinRectangeHeight, toInt);

    PARSE_IF_NOT_EXIST2(GaussianBlurIndexBegin, toInt);
    PARSE_IF_NOT_EXIST2(GaussianBlurIndexEnd, toInt);
}

void parse(CMoveDetectBackgroundSubtractorConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    parse((CMoveDetectConfig&) config, xml);
    PARSE2(HistoryLength, 5, toInt);
}

void parse(CConnectionSettings& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(TCPPort, 0, toUShort);
    PARSE2(UDPPort, 0, toUShort);

    PARSE2(TimeOut, 4, toInt);
    PARSE2(IsCheckHash, false, toInt);
}

void parseDBConfig(NDB::CDBConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(UserName, "", toString);
    PARSE2(Password, "", toString);
    PARSE2(DbHost, "", toString);
    PARSE2(DataBaseName, "", toString);
    PARSE2(DbType, "", toString);
    PARSE2(Port, 0, toInt);
}

void splitArray(
        std::vector<std::string>& array,
        const QXmlStreamAttributes& attributes,
        const std::string& name)
{
    std::string strFromXML;
    PARSE(attributes, name.c_str(), strFromXML,
          "", toString().toStdString);
    QStringList qlist = QString::fromStdString(strFromXML).
            split(";", QString::SkipEmptyParts);

    foreach(QString str, qlist)
    {
        str = str.replace("\n", "").replace(" ","");
        if (!str.isEmpty())
        {
            array.push_back(str.toStdString());
        }
    }
}

void splitArray(
        std::vector<QString>& array,
        const QXmlStreamAttributes& attributes,
        const std::string& name)
{
    std::string strFromXML;
    PARSE(attributes, name.c_str(), strFromXML,
          "", toString().toStdString);
    QStringList qlist = QString::fromStdString(strFromXML).
            split(";", QString::SkipEmptyParts);

    foreach(QString str, qlist)
    {
        str = str.replace("\n", "").replace(" ","");
        if (!str.isEmpty())
        {
            array.push_back(str);
        }
    }
}

void parse(CFaceDetectConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();

    parse((CAlgoBaseConfig&) config, attributes);
    PARSE2(MinPositiveFrames,  5, toUInt);

    PARSE2(IsProcessFirstOnly, true, toInt);
    PARSE2(MinNeighboursMain, 3, toInt);
    PARSE2(MinNeighboursOther, 3, toInt);
    PARSE2(SizeMain, 50, toInt);
    PARSE2(SizeOther, 10, toInt);

    PARSE2(ScaleFactorMain, 2.0, toDouble);
    PARSE2(ScaleFactorOther, 1.3, toDouble);

    PARSE2(CascadeHaarPrefix, "/usr/share/OpenCV/haarcascades/", toString().toStdString);
    splitArray(config.mCascadesHaarMain, attributes, "CascadesHaarMain");
    splitArray(config.mCascadesHaarOther, attributes, "CascadesHaarOther");

    PARSE2(CascadeLBPPrefix, "/usr/share/OpenCV/lbpcascades/", toString().toStdString);
    splitArray(config.mCascadesLBPMain, attributes, "CascadesLBPMain");
    splitArray(config.mCascadesLBPOther, attributes, "CascadesLBPOther");
}

void parse(CDebugShow& config, QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(IsShowDebugImg, false, toInt);
    PARSE2(IsShowAlgoImages, false, toInt);
    PARSE2(IsWriteInfoToMask, true, toInt);
}

void parse(NWebDownwloader::CWebDownloaderConfigList& webConfig, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    NWebDownwloader::CWebDownloaderConfig baseConfig;

    PARSE0(attributes, "RefreshTimeMS", mRefreshTimeMS, 1000, toInt, baseConfig);
    std::vector<QString> list;
    splitArray(list,attributes, "URL");
    for (const auto& str : list)
    {
        if (!str.isEmpty())
        {
            NWebDownwloader::CWebDownloaderConfig config = baseConfig;
            config.mUrl = str;
            webConfig.mWebDownloaders.push_back(config);
        }
    }
}

void parseOnePattern(NText::CTextPatterns& patternsConfig, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    static_assert(std::is_same<NText::CTextPattern::tCeserialData, float>::value, "tCeserialData != float");

    NText::CTextPattern baseConfig;
    PARSE0(attributes, "IsReverse", mIsReverse, false, toUInt, baseConfig);
    PARSE0(attributes, "IsProcess", mIsProcess, false, toUInt, baseConfig);
    PARSE0(attributes, "IsAddToDB", mIsAddToDB, false, toUInt, baseConfig);
    if (baseConfig.mIsProcess)
    {
        PARSE0(attributes, "TreshholdAlert", mTreshholdAlert, 2, toFloat, baseConfig);
        PARSE0(attributes, "TreshholdWarning", mTreshholdWarning, 1, toFloat, baseConfig);
        PARSE0(attributes, "NormalValue", mNormalValue, 0, toFloat, baseConfig);
    }
    PARSE0(attributes, "DiffNotSave", mDiffNotSave, 0, toFloat, baseConfig);
    PARSE0(attributes, "MaxBufferLastSize", mMaxBufferLastSize, 50, toInt, baseConfig);
    parseArray<size_t>(attributes, "BuffersForWrite", baseConfig.mBuffersForWrite);

    QString patterns;
    PARSE(attributes, "Pattern", patterns, "", toString);
    auto qlist = patterns.split(";", QString::SkipEmptyParts);
    for (const QString& str : qlist)
    {
        NText::CTextPattern patternConfig = baseConfig;
        patternConfig.mPattern = str;
        patternConfig.mPattern = patternConfig.mPattern.replace("\n", "").replace(" ","");
        patternsConfig.mPatterns.push_back(patternConfig);
    }
}

void parsePatternsList(NText::CTextPatterns& patternsConfig, QXmlStreamReader& xml)
{
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == patternListsStr))
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "Pattern")
            {
                parseOnePattern(patternsConfig, xml);
            }
        }
        if (token == QXmlStreamReader::Invalid)
        {
            LOG_ERROR << "error xml " << xml.errorString();
            throw "error xml " + xml.errorString().toStdString(); //TODO:
        }
    }
}

void parse(NSerial::CSerialPortConfig& config, const QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    PARSE2(SerialPort, "", toString);
    PARSE2(BaudRate, 9600, toInt);

    PARSE0(attributes, "AlertReplyCountCoef", mAlertMinIntervalRepeatSendCoef, 1, toInt, config.mAlertCoef);
    PARSE0(attributes, "AlertMinIntervalRepeatSendCoef", mAlertMinIntervalRepeatSendCoef, 1, toFloat, config.mAlertCoef);
}

void parseInput(NVideoInput::CInputConfig& inputConfig, const NVideoInput::CInputConfigEntry configBase, QXmlStreamReader& xml)
{
    std::vector<NVideoInput::CInputConfigEntry> inputUrlList;
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == inputStr))
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "CameraInput")
            {
                NVideoInput::CInputConfigEntry configEntry = configBase;
                configEntry.mType = NVideoInput::InputType::CAMERA;
                if (parseInputConfigEntry(configEntry, xml))
                    inputConfig.mInputList.push_back(configEntry);
            }
            else if (xml.name() == "FileInput")
            {
                NVideoInput::CInputConfigEntry configEntry = configBase;
                configEntry.mType = NVideoInput::InputType::FILE;
                if (parseInputConfigEntry(configEntry, xml))
                    inputConfig.mInputList.push_back(configEntry);
            }
            else if (xml.name() == "NetInput")
            {
                std::vector<NVideoInput::CInputConfigEntry> configEntrys;
                if (parseInputURLList(configEntrys, configBase, xml))
                {
                    inputUrlList.insert(inputUrlList.end(), configEntrys.begin(), configEntrys.end());
                }
            }
        }
        if (token == QXmlStreamReader::Invalid)
        {
            LOG_ERROR << "error xml " << xml.errorString();
            throw "error xml " + xml.errorString().toStdString(); //TODO:
        }
    }

    inputConfig.mInputList.insert(inputConfig.mInputList.end(),inputUrlList.begin(),  inputUrlList.end());
    LOG_INFO << inputConfig;
}

void parseSMTPConfig(NAlarms::CSMTPAlertConfig& config, const QXmlStreamReader& xml, const NAlarms::CAlertConfig& baseConfig)
{
    QXmlStreamAttributes attributes = xml.attributes();
    NAlarms::CAlertConfig& alertConf = config;
    alertConf = baseConfig;

    PARSE2(SMTPHost, "", toString);
    PARSE2(SMTPPort, 587, toInt);
    PARSE2(UserName, "", toString);
    PARSE2(UserPassword, "", toString);
    PARSE2(SubjectPrefix, "", toString().toStdString);
    PARSE2(ComputerName, "y-c", toString().toStdString);

    //    PARSE2(ReceiverEMail, "", toString().toStdString);
    QString receiverEMail;
    PARSE(attributes, "ReceiverEMails", receiverEMail, "", toString);
    config.mReceiverEMails = receiverEMail.split(";", QString::SkipEmptyParts);

    PARSE2(SendFrom, 12, toInt);
    PARSE2(SendTo, 19, toInt);
    PARSE2(ZipPassword, "", toString().toStdString);
    PARSE2(LastImagesCountToEmailInZip, 2, toUInt);
    PARSE2(LastImagesCountToEmailEncrypted, 2, toUInt);
    PARSE2(RunExternalCommands, "", toString().toStdString);
    PARSE2(TmpDir, "/var/tmp", toString().toStdString);
    PARSE2(IsWriteVideoToEmail, true, toInt);
    PARSE2(VideoFps, 3.0, toDouble);
    PARSE2(MaskCountToVideo, 2, toInt);
}
} //namespace

void CXMLParser::parseXMLFromFile(
        const std::string& fileName,
        CConfig& config,
        CAlgoConfig& algoConfig,
        CArgumentConfig& argConfig)
{
    NVideoInput::CInputConfigEntry configBase;
    NAlarms::CAlertConfig baseAlertConfig;

    QFile file(fileName.c_str());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LOG_ERROR << "can't open file " << fileName;
        throw "xml error"; //TODO:
    }


    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            LOG_DEBUG << "parse" << xml.name();

            if (xml.name() == "moveDetectBase")
            {
                parseBase(*algoConfig.mMoveDetectConfigColor, xml);
                parseBase(*algoConfig.mMoveDetectConfig, xml);
                parseBase(*algoConfig.mMoveDetectBitwiseAnd, xml);
                parseBase(*algoConfig.mMoveDetectBackgroundSubtractorTestConfig, xml);
                parseBase(*algoConfig.mMoveDetectBackgroundSubtractorTestColorConfig, xml);
            }
            else if (xml.name() == "BaseInput")
            {
                parseInputBase(configBase, xml);
            }
//            else if (xml.name() == "CameraInput")
//            {
//                NVideoInput::CInputConfigEntry configEntry = configBase;
//                configEntry.mType = NVideoInput::InputType::CAMERA;
//                if (parseInputConfigEntry(configEntry, xml))
//                    config.mInputConfig->mInputList.push_back(configEntry);
//            }
//            else if (xml.name() == "FileInput")
//            {
//                NVideoInput::CInputConfigEntry configEntry = configBase;
//                configEntry.mType = NVideoInput::InputType::FILE;
//                if (parseInputConfigEntry(configEntry, xml))
//                    config.mInputConfig->mInputList.push_back(configEntry);
//            }

            else if (xml.name() == inputStr)
            {
                parseInput(*config.mInputConfig, configBase, xml);
            }
            else if (xml.name() == "PHash")
            {
                parse(*algoConfig.mPHashConfig, xml);
            }
            else if (xml.name() == "moveDetect")
            {
                parse(*algoConfig.mMoveDetectConfig, xml);
            }
            else if (xml.name() == "moveDetectColor")
            {
                parse(*algoConfig.mMoveDetectConfigColor, xml);
            }
            else if (xml.name() == "moveDetectBitwiseAnd")
            {
                parse(*algoConfig.mMoveDetectBitwiseAnd, xml);
            }
            else if (xml.name() == "movedetectBackgroundSubtractorTest")
            {
                parse(*algoConfig.mMoveDetectBackgroundSubtractorTestConfig, xml);
            }
            else if (xml.name() == "movedetectBackgroundSubtractorTestColor")
            {
                parse(*algoConfig.mMoveDetectBackgroundSubtractorTestColorConfig, xml);
            }
            else if (xml.name() == "faceDetect")
            {
                parse(*algoConfig.mFaceDetectConfig, xml);
            }
            else if (xml.name() == "BaseAlert")
            {
                parseBaseAlert(baseAlertConfig, xml);
            }
            else if (xml.name() == "CommonAlert")
            {
                parseCommonAlert(*config.mCommonAlertConfig, baseAlertConfig, xml);
            }
            else if (xml.name() == "connection")
            {
                parse(*config.mConnectionSettings, xml);
            }
            else if (xml.name() == "secure")
            {
                parse(*config.mCryptionConfig, xml);
            }
            else if (xml.name() == "DB")
            {
                parseDBConfig(*config.mDataBaseConfig, xml);
            }
            else if (xml.name() == "MJPEGServer")
            {
                parse(*config.mMJPEGServerConfig, xml);
            }
            else if (xml.name() == "debugImg")
            {
                parse(config.mDebugShow, xml);
            }
            else if (xml.name() == "Condition")
            {
                parseCondtion(*config.mConditionList, xml);
            }
            else if (xml.name() == commandsStr)
            {
                parseCommands(*config.mCommandMap, xml);
            }
            else if (xml.name() == "Serial")
            {
                std::shared_ptr<NSerial::CSerialPortConfig> conf(new NSerial::CSerialPortConfig);
                parse(*conf, xml);
                config.mSerialPortsConfig.push_back(conf);
            }
            else if (xml.name() == patternListsStr)
            {
                parsePatternsList(*config.mSerialPatterns, xml);
            }
            else if (xml.name() == "runExternalCommandsOnStart")
            {
                QXmlStreamAttributes attributes = xml.attributes();
                PARSE1(attributes, "value", mRunExternalCommandOnStart,
                       "sudo modprobe video ; sudo modprobe uvcvideo", toString().toStdString);
            }
            else if (xml.name() == "AlgoConfing")
            {
                QXmlStreamAttributes attributes = xml.attributes();
                PARSE1(attributes, "IsASync", mIsAlgoRunAsync,
                       true, toInt);
            }
            else if (xml.name() == "WebDownloader")
            {
                parse(*config.mWebDownloaders, xml);
            }
        }

        if (token == QXmlStreamReader::Invalid)
        {
            LOG_ERROR << "error xml " << xml.errorString();
            throw "error xml " + xml.errorString().toStdString(); //TODO:
        }
    }
    file.close();

    LOG_INFO << *config.mSerialPatterns;

    if (argConfig.mIsVisulize)
    {
        LOG_INFO << "set IsShowDebugImg to true";
        config.mDebugShow.mIsShowDebugImg = true;
    }
    if (argConfig.mIsVisulizeAlgoImages)
    {
        LOG_INFO << "set IsShowAlgoImages to true";
        config.mDebugShow.mIsShowAlgoImages = true;
    }
    if (argConfig.mIsDebug)
    {
        argConfig.mNotNotUseSMTP = true;
        config.mCommonAlertConfig->mFinalVideoConfig->mSaveFinalVideoPaths.clear();
        config.mCommonAlertConfig->mSMTPConfig->mSMTPHost.clear();
        config.mCommonAlertConfig->mSMTPConfig->mUserName.clear();
        config.mCommonAlertConfig->mSMTPConfig->mUserPassword.clear();
    }
    if (argConfig.mNotNotUseSMTP)
    {
        config.mCommonAlertConfig->mSMTPConfig->mSMTPHost.clear();
        config.mCommonAlertConfig->mSMTPConfig->mUserName.clear();
        config.mCommonAlertConfig->mSMTPConfig->mUserPassword.clear();
        config.mCommonAlertConfig->mTelegramConfig->mToken.clear();
        LOG_INFO << "no send SMTP";
    }
    if (argConfig.mNotUseThreads)
    {
        config.mIsAlgoRunAsync = false;
        LOG_INFO << "IsAlgoRunAsync = true";
    }
}
