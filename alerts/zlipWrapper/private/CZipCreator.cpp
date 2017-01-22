#include <fstream>
#include <minizip/zip.h>

#include "alerts/zlipWrapper/CZipCreator.hpp"
#include "logging/Log.hpp"
namespace NZIP
{
bool CZipCreator::createZipFile(
        const std::string& archiveName,
        const char* archPassword,
        const std::vector<std::string>& paths)
{
    zipFile zf = zipOpen(archiveName.c_str(), APPEND_STATUS_CREATE);
    if (zf == NULL)
    {
        LOG_ERROR << "fail zipOpen" << archiveName;
        return false;
    }

    bool retVal = true;
    //TODO: foreach
    for (size_t i = 0; i < paths.size(); i++)
    {
        uLong crc = crc32(0L, Z_NULL, 0);
        std::fstream file(paths[i].c_str(), std::ios::binary | std::ios::in);
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
            long size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<char> buffer(size);
            if (size == 0 || file.read(&buffer[0], size))
            {
                zip_fileinfo zfi = { 0 };
                std::string fileName = paths[i];
                fileName = fileName.substr(paths[i].rfind('\\')+1);
                fileName = fileName.substr(paths[i].rfind('/')+1);

                const unsigned char* buf = reinterpret_cast<const unsigned char*>(buffer.data());
                crc = crc32(crc, buf, buffer.size());

                int retValue =zipOpenNewFileInZip3(
                            zf,
                            fileName.c_str(),
                            &zfi,
                            NULL,
                            0,
                            NULL,
                            0,
                            NULL,
                            Z_DEFLATED,
                            Z_DEFAULT_COMPRESSION,
                            0,
                            -MAX_WBITS,
                            DEF_MEM_LEVEL,
                            Z_DEFAULT_STRATEGY,
                            archPassword,
                            crc
                            );

                if (0 == retValue)
                {
                    if (zipWriteInFileInZip(zf, size == 0 ? "" : &buffer[0], size))
                    {
                        LOG_WARN << "fail zipWriteInFileInZip";
                        retVal = false;
                    }

                    if (zipCloseFileInZip(zf))
                    {
                        LOG_WARN << "fail zipCloseFileInZip";
                        retVal = false;
                    }

                    file.close();
                    continue;
                }
                else
                {
                    LOG_WARN << "zipOpenNewFileInZip3 fail" << fileName << " ret value " << retValue;
                }
            }
            else
            {
            }
            file.close();
        }
        else
        {
            LOG_WARN <<"zip open fail " << paths[i];
        }
        retVal = false;
    }

    if (zipClose(zf, NULL))
    {
        LOG_ERROR << "fail zipClose";
        //return false?
    }

    if (!retVal)
    {
        LOG_ERROR << "return fail";
        return false;
    }
    return true;
}
}
