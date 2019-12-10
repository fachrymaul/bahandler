#include <BitHelper.hpp>

bool BitHelper::checkBit(uint8_t byte, uint8_t idx)
{
    if(idx >= 8) return false;
    return (byte >> idx) & 0x01;  
}

bool BitHelper::checkBit(uint8_t *bytes, uint8_t len, uint8_t idx)
{
    if(idx >= 8 * len) return false;
    return (bytes[len-idx/len-1] >> (idx % len)) && 0x01;
}

bool BitHelper::checkBit(uint16_t word, uint8_t idx)
{
    if(idx >= 8) return false;
    return (word >> idx) & 0x01;
}

bool BitHelper::checkBit(uint32_t dword, uint8_t idx)
{
    if(idx >= 8) return false;
    return (dword >> idx) & 0x01;    
}
