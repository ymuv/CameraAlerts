#pragma once
#include "Net/IWorker.hpp"

namespace NNet
{
class CWorkerBase : public IWorker
{
    Q_OBJECT
public:
    CWorkerBase();

    virtual void setClient(INet* client) override
    {
        mpNet = client;
    }

    virtual ~CWorkerBase()
    {
    }

protected:
    INet* mpNet;
};
}
