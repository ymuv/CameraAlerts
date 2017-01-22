#include <ostream>

#include "Image/motionAlgorithms/CMoveDetectConfig.hpp"

#define PRINT_VAR_TO_STREAM_FROM_CONFIG(stream, varName, configName) \
   stream << varName << configName << ";"

#define PRINT_VAR_TO_STREAM_FROM_CONFIG2(var) \
   PRINT_VAR_TO_STREAM_FROM_CONFIG(stream, #var"=", config.m##var)

std::ostream& operator<<(std::ostream& stream, const CMoveDetectConfig& config)
{
    stream << " [";
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(IsRun);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(AreaMinSize);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(MinRectangeWidth);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(MinRectangeHeight);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(MinTrashHold);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(SkipFrames);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(IsUseErode);
    PRINT_VAR_TO_STREAM_FROM_CONFIG2(IsUseDilate);
    stream << "]"; //TODO: new values!!
    return stream;
}
