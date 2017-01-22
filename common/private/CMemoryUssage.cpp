#include <string.h>
#include <string>

#include "common/CMemoryUssage.hpp"

#ifdef __linux__

int parseLine(char* line) //TODO: return bool, syntax: bool parseLine(char* line, char*/string pattern)
{
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}
int CMemoryUssage::processMemoryUssage(int& maxMemoryUssage)
{
    using std::ios_base;
    using std::ifstream;
    using std::string;

    maxMemoryUssage = 0.0;

    FILE* file = fopen("/proc/self/status", "r");
    if (file)
    {
        int currentUssage = 0;
        char line[128];

        while (fgets(line, 128, file) != NULL)
        {
            if (strncmp(line, "VmRSS:", 6) == 0)
            {
                currentUssage = parseLine(line);
            }

            if (strncmp(line, "VmHWM:", 6) == 0)
            {
                maxMemoryUssage = parseLine(line);
            }
        }
        fclose(file);
        static int KB_IN_MB = 1024;
        maxMemoryUssage = maxMemoryUssage / KB_IN_MB;
        return currentUssage / KB_IN_MB;
    }
    maxMemoryUssage = 0;
    return 0;
}

#elif _WIN32
#include <windows.h>
#include <psapi.h>
int CMemoryUssage::processMemoryUssage(int& maxMemoryUssage)
{
    maxMemoryUssage = 0;
    PROCESS_MEMORY_COUNTERS pmc;

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                                   GetCurrentProcessId());
    if (NULL == hProcess)
    {
        maxMemoryUssage = 0;
        return 0;
    }

    if (GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)))
    {
        static int BYTE_TO_MB = 1024 * 1024;
        maxMemoryUssage = pmc.PeakWorkingSetSize  / BYTE_TO_MB;
        return pmc.WorkingSetSize / BYTE_TO_MB;  //to MB;
    }
    maxMemoryUssage = 0;

    return 0;
}
#endif

int CMemoryUssage::processMemoryUssage()
{
    int dummy;
    return processMemoryUssage(dummy);
}

int CMemoryUssage::processMaxMemoryUssage()
{
    int maxMemoryUssage;
    processMemoryUssage(maxMemoryUssage);
    return maxMemoryUssage;
}
