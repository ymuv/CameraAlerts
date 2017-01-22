#pragma once

//Not used
class CMD4
{
    CMD4() = delete;
public:
    template <class T>
    static T calcMD4(const T& data, int offset, int endOffset);
};
