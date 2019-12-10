#ifndef BIT_HELPER_H
#define BIT_HELPER_H

#include <cstdint>

class BitHelper
{
  public:
    static bool checkBit(uint8_t byte, uint8_t idx);
    static bool checkBit(uint8_t *bytes, uint8_t len, uint8_t idx);
    static bool checkBit(uint16_t word, uint8_t idx);
    static bool checkBit(uint32_t dword, uint8_t idx);
};

#endif