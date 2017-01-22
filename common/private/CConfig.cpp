#include "Image/videoInput/CInputConfig.hpp"
#include "MJPEGServer/CMJPEGServerConfig.hpp"
#include "Net/CNetCryptionConfig.hpp"
#include "Text/CTextPatterns.hpp"
#include "TextToSpeech/CTextToSpeechConfig.hpp"
#include "alerts/CCommonAlertConfig.hpp"
#include "alerts/CSMTPAlertConfig.hpp"
#include "common/CConnectionSettings.hpp"
#include "common/CConfig.hpp"
#include "conditions/CCondition.hpp"
#include "command/CCommand.hpp"
#include "db/CDBConfig.hpp"
#include "gui/CGuiConfig.hpp"
#include "serial/CSerialPortConfig.hpp"
#include "webDownload/CWebDownloaderConfig.hpp"

CConfig& CConfig::getInstance()
{
    static CConfig sConfigInstance;
    return sConfigInstance;
}

CConfig::CConfig()
{
    mInputConfig.reset(new NVideoInput::CInputConfig);
    mCryptionConfig.reset(new NNet::CNetCryptionConfig);
    mConnectionSettings.reset(new CConnectionSettings);
    mMJPEGServerConfig.reset(new NMJPEG::CMJPEGServerConfig);
    mGuiConfig.reset(new CGuiConfig);
    mSerialPatterns.reset(new NText::CTextPatterns);
    mWebDownloaders.reset(new NWebDownwloader::CWebDownloaderConfigList);

    mCommonAlertConfig.reset(new NAlarms::CCommonAlertConfig);
    mConditionList.reset(new NCondition::CConditionList);
    mCommandMap.reset(new NCommand::CCommandsMap);
    mDataBaseConfig.reset(new NDB::CDBConfig);
    mSpeechConfig.reset(new NTextToSpeach::CTextToSpeechConfig);
}

#define PRINT_VAR_TO_STREAM_FROM_CONFIG(stream, varName, configName) \
    stream << varName << configName << ";"

#define PRINT_VAR_TO_STREAM_FROM_CONFIG2(var) \
    PRINT_VAR_TO_STREAM_FROM_CONFIG(stream, #var"=", config.m##var)

std::ostream& operator<<(std::ostream& stream, const NVideoInput::CInputConfigEntry& config)
{
    using namespace NVideoInput;
    stream << " [";
    if (config.mType == InputType::CAMERA)
    {
        stream << "cam:" + std::to_string(config.mCameraId)+ ";";
        if (config.mFrameHeight > 0)
            PRINT_VAR_TO_STREAM_FROM_CONFIG2(FrameHeight);
        if (config.mFrameWidth > 0)
            PRINT_VAR_TO_STREAM_FROM_CONFIG2(FrameWidth);
    }
    else if (config.mType == InputType::FILE)
    {
        stream << "file:" + config.mFile+ ";";
    }
    else if (config.mType == InputType::URL)
    {
        stream << "URL: " << ";";
        stream << NNet::toString(config.mConnectionConfig.mTypeProtocol);
        stream << "_" << config.mConnectionConfig.mServerHost + "_" + std::to_string(config.mConnectionConfig.mPort)
                + "_image_id" + std::to_string(static_cast<int>(config.mConnectionConfig.mRequestdImageId))
                +  "_id" + std::to_string(config.mConnectionConfig.mImageId) << "; ";
    }
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(BufferSize);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(LongBufferSize);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(LongBufferSkip);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(ResizeScaleFactor);

    PRINT_VAR_TO_STREAM_FROM_CONFIG2(ResizeScaleFactor);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(ResizeScaleFactorForNet);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(SleepTime);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(BufferSize);
    stream << "]\n";

    return stream;
}

std::ostream&operator<<(std::ostream& stream, const NVideoInput::CInputConfig& inputConfig)
{
    stream << "[\n";
    for (size_t i = 0; i < inputConfig.mInputList.size(); i++)
    {
        stream <<inputConfig.mInputList[i];
    }
    stream << "]\n";
    return stream;
}


std::ostream& operator<<(std::ostream& stream, const CConfig& config)
{
    stream << "Config:[\n";
    stream << *config.mInputConfig;
    stream << "]";
    return stream;
}
