#pragma once

class CCRC
{
    CCRC() = delete;
public:
    template <class T>
    static unsigned long calc(const T& data, int offset, int endOffset);
};
