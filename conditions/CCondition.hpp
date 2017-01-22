#pragma once

#include <QString>
#include <QMap>

namespace NCondition
{

struct CCondition
{
    QString mExpression;
    QString mAlert;
    QString mCommand;
};

struct CConditionList
{
    std::vector<CCondition> mConditions;
};
}
