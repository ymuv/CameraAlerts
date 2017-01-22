#pragma once

#include <string>
#include <vector>

namespace NZIP
{
class CZipCreator
{
public:
    static bool createZipFile(
            const std::string& archiveName,
            const char* archPassword,
            const std::vector<std::string>& paths);
};
}
