#include <Arduino.h>
#include <../lib/MDBMaster/inc/MDBMaster.hpp>

SerialEsp serialesp;
MDBMaster mdb;
void setup()
{
    Serial2.begin(19200);
    serialesp.serialInit(&Serial2);
    mdb.setup(&serialesp);
    delay(100);
}

void loop()
{
    mdb.acceptMoney();
    delay(100);
}
