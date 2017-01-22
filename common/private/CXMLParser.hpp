#pragma once

#include <string>

struct CConfig;
struct CAlgoConfig;

struct CArgumentConfig
{
    bool mIsVisulize = false;
    bool mIsVisulizeAlgoImages = false;
    bool mIsDebug = false;
    std::string mFileName = "";
    bool mIsUseProfiler;
    bool mNotNotUseSMTP;
    bool mNotUseThreads;
};

class CXMLParser
{
public:
    static void parseXMLFromFile(
            const std::string& fileName,
            CConfig& config,
            CAlgoConfig& algoConfig,
            CArgumentConfig& argConfig);
};

