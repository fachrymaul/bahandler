#include <MDBProtocolTermiosUSB.hpp>
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
    MDBProtocolTermiosUSB mdbProto;
    Linux::TermiosSerialUSB term;
    Linux::MillisTimer comTimer;

    comTimer.start();

    if (term.openComMDB(devPort,
                        Linux::TermiosSerialUSB::BAUD_9600,
                        Linux::TermiosSerialUSB::PAR_SPACEMARK,
                        Linux::TermiosSerialUSB::STOPBITS_ONE))

    {
        bv.initialize(&mdbProto, &term, &comTimer);

        // while (!bv.reset())
        // {
        //     // Wait 10 s retry
        //     comTimer.delayMs(10000);
        // }

        bv.reset();

        uint64_t startMillis = comTimer.getMillis();
        debug.setDebug(false);

        while (true)
        {
            bv.update();
            if (comTimer.getMillis() - startMillis > 3000)
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
