#pragma once

class CMemoryUssage
{
public:
    static int processMemoryUssage(int& maxMemoryUssage);
    static int processMemoryUssage();
    static int processMaxMemoryUssage();

    CMemoryUssage() = delete;
};
