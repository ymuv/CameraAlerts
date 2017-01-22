#include "common/CMainFunctions.hpp"
#include "logging/Log.hpp"
#include "serial/CSerialPortReader.hpp"

namespace NSerial
{

void CSerialPortReader::openSerial()
{
    if (!mSerial.open(QIODevice::ReadOnly)) {
        LOG_WARN << "fail open port" << mConfig.mSerialPort << mSerial.error() << mSerial.errorString();
    }
    else
    {
        connect(&mSerial, SIGNAL(readyRead()), this, SLOT(handleRead()));
        connect(&mSerial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                this, &CSerialPortReader::handleError);
    }
}

void CSerialPortReader::write(const QByteArray& value)
{
    mSerial.write(value);
}

CSerialPortReader::CSerialPortReader(const CSerialPortConfig& config)
    : mSerial(this) //TODO: test it
    , mConfig(config)
    , mAlertConfig("serial_port", mConfig.mAlertCoef)
{
    mSerial.setPortName(config.mSerialPort);
    mSerial.setBaudRate(config.mBaudRate);
    const int TIMER_DEFAULT_INTERVAL = 5000;
    mTimer.setInterval(TIMER_DEFAULT_INTERVAL);
    openSerial();

    connect(&mTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    mTimer.start();
}

CSerialPortReader::~CSerialPortReader()
{
}


void CSerialPortReader::handleRead()
{
    mTimer.stop();
    CMainFunctions::doTextAlgo(mSerial.readAll(), mAlertConfig);
    mTimer.start();
}

void CSerialPortReader::handleTimeout()
{
    if (!mSerial.isOpen())
    {
        openSerial();
    }
}

void CSerialPortReader::handleError(QSerialPort::SerialPortError serialPortError)
{
    LOG_WARN << serialPortError << mSerial.errorString(); //crash here
    mSerial.close(); //close here?
}
}
