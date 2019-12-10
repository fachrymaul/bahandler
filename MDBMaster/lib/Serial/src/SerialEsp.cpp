#include "../inc/SerialEsp.hpp"

SerialEsp::SerialEsp()
{
    m_serial = nullptr;
}

bool SerialEsp::available()
{
    return m_serial->available();
}

bool SerialEsp::serialInit(HardwareSerial *serial)
{
    m_serial = serial;
    return true;
}

bool SerialEsp::write(uint8_t data)
{
    return m_serial->write(data);
}

bool SerialEsp::read()
{
    return m_serial->read();
}