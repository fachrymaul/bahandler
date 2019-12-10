#include "catch.hpp"
#include "fakeit.hpp"
#include "CRC16.hpp"
using namespace fakeit;

TEST_CASE("CRC16-check")
{
    CRC16 crc16;

    SECTION("CRC16-Function-Check-Success")
    {
        uint8_t data[3] = {0xfe,0x04,0x00};
        REQUIRE( crc16.crc16_ccitt((char*)data,3) == 0xf80b);
        data[0] = 0xff;
        data[1] = 0xff;
        data[2] = 0xff;
        REQUIRE( crc16.crc16_ccitt((char*)data,3) == 0x1EF0);    
    }

    SECTION("CRC16-Function-Check-Failed")
    {
        uint8_t data[3] = {0xfe,0x04,0x00};
        data[0] = 0xff;
        data[1] = 0xff;
        data[2] = 0xff;
        REQUIRE_FALSE( crc16.crc16_ccitt((char*)data,3) == 0x0001);       
    }


}