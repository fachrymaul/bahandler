#include <iostream>
#include "peripheral/inc/Serial.hpp"
#include <unistd.h>
#include "MDB/inc/BillAcceptor.hpp"
#include "peripheral/inc/LinuxMillisTimer.hpp"

using namespace Linux;

int main()
{
    std::cout << "hello world" << std::endl;
    Serial serObj;
    MillisTimer milis;
    BillAcceptor validator(&serObj, &milis);

    int status = serObj.initialize("/dev/ttyS6");

    if (status < 0)
    {
        return -1;
    }

    // uint8_t buff[32] = {0};
    // uint32_t rxlength = 0;

    // validator.setup();

    // while (1)
    // {
    //     if (serObj.receive(buff, 1, &rxlength) > -1)
    //     {
    //         serObj.transmit(buff, 1);
    //     }
    // }

    while(1)
    {
        validator.listen();
    }

    return 0;
}