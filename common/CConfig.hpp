#pragma once

#include <memory>
#include <vector>
#include "CRunStatus.hpp"

struct CGuiConfig;
struct CDebugShow;
struct CConnectionSettings;
struct CAlertsStatus;

namespace NVideoInput
{
    struct CInputConfig;
}

namespace NSerial
{
    struct CSerialPortConfig;
}

namespace NText
{
    struct CTextPatterns;
}

namespace NWebDownwloader
{
    struct CWebDownloaderConfigList;
}

namespace NNet
{
    struct CNetCryptionConfig;
}

namespace NCommand
{
    struct CCommandsMap;
}

namespace NMJPEG
{
    struct CMJPEGServerConfig;
}
namespace NCondition
{
    struct CConditionList;
}
namespace NTextToSpeach
{
    struct CTextToSpeechConfig;
}

namespace NAlarms
{
    struct CSMTPAlertConfig;
    struct CCommonAlertConfig;
}
namespace NDB
{
    struct CDBConfig;
}

struct CDebugShow
{
    bool mIsShowDebugImg = true;
    bool mIsShowAlgoImages;
    bool mIsWriteInfoToMask;
};

struct CConfig
{
    std::shared_ptr<NVideoInput::CInputConfig>  mInputConfig;

    std::shared_ptr<NNet::CNetCryptionConfig> mCryptionConfig;
    std::shared_ptr<NAlarms::CCommonAlertConfig> mCommonAlertConfig;

    std::shared_ptr<CConnectionSettings> mConnectionSettings;
    std::shared_ptr<NMJPEG::CMJPEGServerConfig> mMJPEGServerConfig;

    std::shared_ptr<CGuiConfig> mGuiConfig;
    std::vector<std::shared_ptr<NSerial::CSerialPortConfig>> mSerialPortsConfig;

    std::shared_ptr<NWebDownwloader::CWebDownloaderConfigList> mWebDownloaders;
    std::shared_ptr<NText::CTextPatterns> mSerialPatterns;
    std::shared_ptr<NCondition::CConditionList> mConditionList;
    std::shared_ptr<NCommand::CCommandsMap> mCommandMap;
    std::shared_ptr<NDB::CDBConfig> mDataBaseConfig;
    std::shared_ptr<NTextToSpeach::CTextToSpeechConfig> mSpeechConfig;

    CRunStatus mRunStatus;
    CDebugShow mDebugShow;

    std::string mRunExternalCommandOnStart; //use for modprobe
    bool mIsAlgoRunAsync = true;

public: //static methods
    static CConfig& getInstance();

private: //methods
    CConfig();
    CConfig(CConfig&) = delete;
    CConfig(CConfig&&) = delete;
    CConfig& operator=(CConfig const&) = delete;
    CConfig& operator=(CConfig const&&) = delete;
};

std::ostream& operator<<(std::ostream& stream, const CConfig&);
