#include "serial/CSerialPorts.hpp"
#include "serial/CSerialPortReader.hpp"

namespace NSerial
{

NSerial::CSerialPorts& NSerial::CSerialPorts::getInstance()
{
    static CSerialPorts instance;
    return instance;
}

void CSerialPorts::addPort(const CSerialPortConfig& config)
{
    std::shared_ptr<NSerial::CSerialPortReader> reader(
                new NSerial::CSerialPortReader(config));
    mPorts.push_back(reader);
}

CSerialPorts::CSerialPorts()
{
}

const std::vector<std::shared_ptr<CSerialPortReader>>& CSerialPorts::getPorts() const
{
    return mPorts;
}
}
