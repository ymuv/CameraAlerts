#pragma once

#include <memory>

namespace NNet
{
class INet;

class CAES
{
public:

    template<class T>
    static void crypt(const T& str, INet* net);

    template<class T>
    static void decrypt(const T& str, INet* net);

private:
    CAES() = delete;
};

}
