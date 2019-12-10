#include <MDBProtocolConv.hpp>
#include <MDBProtocolTermiosDummy.hpp>
#include <TermiosSerialDummy.hpp>
#include <TermiosSerialUSB.hpp>
#include <LinuxMillisTimer.hpp>
#include <BillValidatorMDB.hpp>
// #include <BillValidatorICT.hpp>
// #include <ICTProtocol.hpp>
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
    // BillValidatorICT bv;
    // ICTProtocol ICTProto;
    MDBProtocolConv mdbProtoConv;
    MDBProtocolTermiosDummy mdbProtoTermiosDummy;
    Linux::TermiosSerialUSB term;
    // Linux::TermiosSerialDummy term;
    Linux::MillisTimer comTimer;

    comTimer.start();

    // if (term.openComMDB(devPort, Linux::TermiosSerialDummy::BAUD_9600))
    // if (term.openComMDB(devPort,
    //                     Linux::TermiosSerialUSB::BAUD_9600,
    //                     Linux::TermiosSerialUSB::PAR_EVEN,
    //                     Linux::TermiosSerialUSB::STOPBITS_ONE))
    if (term.openComMDB(devPort,
                        Linux::TermiosSerialUSB::BAUD_9600,
                        Linux::TermiosSerialUSB::PAR_NONE,
                        Linux::TermiosSerialUSB::STOPBITS_ONE))

    {
        // bv.initialize(&ICTProto, &term, &comTimer);
        bv.initialize(&mdbProtoConv, &term, &comTimer);
        bv.reset();

        uint64_t startMillis = comTimer.getMillis();
        debug.setDebug(false);

        while (true)
        {
            bv.update();
            if(comTimer.getMillis() - startMillis > 3000)
            {
                std::cout << "Credits: " << bv.getCredit() << "\n";
                startMillis = comTimer.getMillis();
            }
        }

        /*
        while (!bv.reset())
        {
            // Wait 10 s retry
            comTimer.delayMs(10000);
        }

        while (1)
        {
            bv.update();
            debug << "BV: Credits = " << bv.getCredit() << "\n";
            comTimer.delayMs(1000);
        }
        */

        term.closeComMDB();
    }

    debug << "Bye"
          << "\n";

    return 0;
}
