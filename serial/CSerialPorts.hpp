#pragma once

#include <memory>
#include <vector>

namespace NSerial
{
class CSerialPortReader;
struct CSerialPortConfig;

class CSerialPorts
{
public:
    static CSerialPorts& getInstance();

    void addPort(const CSerialPortConfig& config);
    const std::vector<std::shared_ptr<CSerialPortReader>>& getPorts() const;

private:
    CSerialPorts();

private:
    std::vector<std::shared_ptr<CSerialPortReader>> mPorts;
};
} //namespace
