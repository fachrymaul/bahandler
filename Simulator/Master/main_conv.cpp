#include <MDBProtocolConv.hpp>
#include <MDBProtocolTermiosDummy.hpp>
#include <TermiosSerialDummy.hpp>
#include <TermiosSerialUSB.hpp>
#include <LinuxMillisTimer.hpp>
#include <BillValidatorMDB.hpp>
#include <iostream>

int main(int argc, char *argv[])
{

#ifdef DEBUG
    debug.setDebug(true);
#else
    debug.setDebug(false);
#endif

    debug << "MDB Master Simulator"
          << "\n";
    debug << "--------------------"
          << "\n";
    if (argc < 2)
    {
        error << "Use with arg: MDBProtocolMaster <port>"
              << "\n";
        return -1;
    }
    const char *devPort = argv[1];

    // Object declaration
    BillValidatorMDB bv;
    MDBProtocolConv mdbProtoConv;
    MDBProtocolTermiosDummy mdbProtoTermiosDummy;
    Linux::TermiosSerialUSB term;
    Linux::MillisTimer comTimer;

    comTimer.start();

    if (term.openComMDB(devPort,
                        Linux::TermiosSerialUSB::BAUD_9600,
                        Linux::TermiosSerialUSB::PAR_NONE,
                        Linux::TermiosSerialUSB::STOPBITS_ONE))

    {
        debug.setDebug(false);
        error.setDebug(true);
        warning.setDebug(true);
        bv.initialize(&mdbProtoConv, &term, &comTimer);

        while (!bv.reset())
        {
            // Wait 10 s retry
            comTimer.delayMs(10000);
        }

        uint64_t startMillis = comTimer.getMillis();
        // debug.setDebug(false);

        while (true)
        {   
            bv.update();
            if (comTimer.getMillis() - startMillis > 2000)
            {
                std::cout << "Credits: " << bv.getCredit() << "\n";
                startMillis = comTimer.getMillis();
            }
        }

        term.closeComMDB();
    }

    debug << "Bye"
          << "\n";

    return 0;
}
