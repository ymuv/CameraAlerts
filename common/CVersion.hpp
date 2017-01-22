#pragma once
#include <string>

class CVersion
{
public:
   CVersion() = delete;
   static std::string& getApplicationInfo();

   static const char* getProjectBuildVersion();
   static const char* getProjectBuildTime();
   static const char* getProjectLastCommitDateTime();
};

