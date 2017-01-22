#include "common/CVersion.hpp"
#include "generated/version.hpp"

std::string& CVersion::getApplicationInfo()
{
   static std::string sAppInfo = std::string(PROJECT_BUILD_VERSION)
           + " " + std::string(PROJECT_BUILD_TIME)
           + " " + std::string(PROJECT_LAST_COMMIT_DATETIME);
   return sAppInfo;
}

const char*CVersion::getProjectBuildVersion()
{
   return PROJECT_BUILD_VERSION;
}

const char*CVersion::getProjectBuildTime()
{
    return PROJECT_BUILD_TIME;
}

const char*CVersion::getProjectLastCommitDateTime()
{
    return PROJECT_LAST_COMMIT_DATETIME;
}
