#pragma once

#include <QByteArray>
#include <QtSerialPort/QtSerialPort>
#include <QObject>
#include <QTextStream>
#include <QTimer>

#include "CSerialPortConfig.hpp"
#include "common/CAlertNotification.hpp"

namespace NSerial
{
class CSerialPortReader : public QObject
{
    Q_OBJECT
public:
    using Ptr = std::shared_ptr<CSerialPortReader>;

public:
    explicit CSerialPortReader(const CSerialPortConfig& config);
    ~CSerialPortReader();

    void openSerial();
    void write(const QByteArray& value);

private slots:
    void handleRead();
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort mSerial;
    QTimer mTimer;
    const CSerialPortConfig& mConfig;

    CAlertNotification mAlertConfig; //TODO: temp, remove from here after code reorganize
};
}
