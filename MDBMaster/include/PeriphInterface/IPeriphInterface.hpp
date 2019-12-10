#ifndef I_PERIPH_INTERFACE_H
#define I_PERIPH_INTERFACE_H

#include <stdint.h>

class ISerial
{
public:
virtual bool write(uint8_t data);
virtual bool read();
virtual bool available();
};

#endif