#pragma once

#include <memory>

namespace NNet
{
class IServer
{
public:
    virtual void start() = 0;
    virtual ~IServer()
    {}
};

using pServer = std::shared_ptr<IServer>;
}
