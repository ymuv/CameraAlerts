#include "Image/videoInput/CInputConfig.hpp"
#include "Text/CTextPatterns.hpp"
#include "serial/CSerialPortConfig.hpp"

#define PRINT_VAR_TO_STREAM_FROM_CONFIG_QSTR(var) \
    stream << #var"=" + config.m##var + ";"


#define PRINT_VAR_TO_STREAM_FROM_CONFIG_BOOL(var) \
    stream << #var"=" + (config.m##var ? QString("true") : QString("false")) + ";"

#define PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(var) \
    if (config.m##var == config.m##var) \
        stream << #var"=" + QString::number(config.m##var) + ";"

QDebug operator<<(QDebug stream, const NText::CTextPatterns& serialPatternsConfig)
{
    stream << "CSerialPatterns:[\n";
    for (const auto& config : serialPatternsConfig.mPatterns)
    {
        stream << "Pattern:[";
        PRINT_VAR_TO_STREAM_FROM_CONFIG_QSTR(Pattern);
        PRINT_VAR_TO_STREAM_FROM_CONFIG_BOOL(IsProcess);
        PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(TreshholdAlert);
        PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(TreshholdWarning);
        PRINT_VAR_TO_STREAM_FROM_CONFIG_BOOL(IsReverse);
        stream << "];\n";
    }
    stream << "]";
    return stream;
}

QDebug operator<<(QDebug stream, const NSerial::CSerialPortConfig& config)
{
    stream << "SerialPortConfig:[";
    PRINT_VAR_TO_STREAM_FROM_CONFIG_QSTR(SerialPort);
    stream << "]";
    return stream;
}


QDebug operator<<(QDebug stream, const NVideoInput::CInputConfigEntry& config)
{
    using namespace NVideoInput;
    stream << " Input:[";
    if (config.mType == InputType::CAMERA)
    {
        stream << "cam:" + QString::number(config.mCameraId)+ ";";
        if (config.mFrameHeight > 0)
            PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(FrameHeight);
        if (config.mFrameWidth > 0)
            PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(FrameWidth);
    }
    else if (config.mType == InputType::FILE)
    {
        stream << "file:" + QString::fromStdString(config.mFile)+ ";";
    }
    else if (config.mType == InputType::URL)
    {
       stream << "URL: ;"+ QString::fromStdString(NNet::toString(config.mConnectionConfig.mTypeProtocol))
                 + "_" + QString::fromStdString(config.mConnectionConfig.mServerHost) + "_" + QString::number(config.mConnectionConfig.mPort)
                 + "_image_id" +  QString::number(static_cast<int>(config.mConnectionConfig.mRequestdImageId))
                 +  "_id" +  QString::number(config.mConnectionConfig.mImageId) << "; ";
    }
    PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(BufferSize);
    PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(LongBufferSize);
    PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(LongBufferSkip);
    PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(ResizeScaleFactor);

    PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(ResizeScaleFactor);
    PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(ResizeScaleFactorForNet);
    PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(SleepTime);
    PRINT_VAR_TO_STREAM_FROM_CONFIG_NUMBER(BufferSize);
    stream << "]\n";

    return stream;
}

QDebug operator<<(QDebug stream, const NVideoInput::CInputConfig& inputConfig)
{
    stream << "Inputs:[\n";
    for (size_t i = 0; i < inputConfig.mInputList.size(); i++)
    {
        stream <<inputConfig.mInputList[i];
    }
    stream << "]\n";
    return stream;
}
