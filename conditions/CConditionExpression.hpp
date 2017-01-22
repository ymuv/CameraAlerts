#pragma once
#include <QString>
#include <memory>
#include <mutex>

class QJSEngine;
class QJSValue;

namespace NCondition
{
struct CCondition;

class CConditionExpression
{
public: //static methods
    using Ptr  = std::shared_ptr<CConditionExpression>;
    static Ptr getInstance(); //TODO: use &??

public: //methods
    void checkAllExpression();

private:
    CConditionExpression();
    void doCondition(const CCondition& condition, const QString& command);
    void setProperty(const QString& property, const QJSValue& value);
private:
    std::unique_ptr<QJSEngine> mEngine;
    std::mutex mMutex;
};
}
