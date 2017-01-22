#pragma once
#include <QDebug>
#include <QString>
#include <string>
#include <vector>

#include "alerts/CAlertCoefConfig.hpp"

namespace NSerial
{
struct CSerialPortConfig
{
    QString mSerialPort;
    int mBaudRate;
    NAlarms::CAlertCoefConfig mAlertCoef;
};
}
QDebug operator<<(QDebug stream, const NSerial::CSerialPortConfig&);
