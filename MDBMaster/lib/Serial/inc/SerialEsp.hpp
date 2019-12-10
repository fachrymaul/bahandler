#ifndef SERIAL_H
#define SERIAL_H
#include <../../../include/PeriphInterface/IPeriphInterface.hpp>
#include <Arduino.h>

class SerialEsp final : public ISerial 
{
public:
    SerialEsp();
    virtual ~SerialEsp(){};
    bool serialInit(HardwareSerial *serial);
    virtual bool write(uint8_t data);
    virtual bool read();
    virtual bool available();

private:
    HardwareSerial *m_serial;
};

#endif