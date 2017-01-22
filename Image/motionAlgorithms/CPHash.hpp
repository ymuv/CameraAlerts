#pragma once
#include <QList>

#include "CAbstractAlertAlgorighm.hpp"

namespace NAlgorithms
{
struct CPHashConfig;

class CPHash : public CAbstractAlertAlgorighm
{
public:
    CPHash(const CPHashConfig& conf, size_t bufferId);
    virtual const std::string getAlgorithmName() const override;

protected:
    virtual bool hasAlert(const QList<std::shared_ptr<CMatWithTimeStamp> >& buff) override;

private:
    int64 pHashValue(const cv::Mat& src);

    int64 calcHammingDistance(int64 x, int64 y) const;
private:
    const CPHashConfig& mConfig;
};
}
