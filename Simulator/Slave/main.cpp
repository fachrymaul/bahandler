#include <MDBProtocolConv.hpp>
#include <MDBProtocolTermiosDummy.hpp>
#include <TermiosSerialDummy.hpp>
#include <LinuxMillisTimer.hpp>
#include <BillValidator.hpp>

int main(int argc, char *argv[])
{

#ifdef DEBUG
    debug.setDebug(true);
#else
    debug.setDebug(false);
#endif

    debug << "MDB Slave Simulator"
          << "\n";
    debug << "--------------------"
          << "\n";
    if (argc < 2)
    {
        error << "Use with arg: MDBProtocolSlave <port>"
              << "\n";
        return -1;
    }
    const char *devPort = argv[1];

    // Object declaration
    BillValidator bv;
    MDBProtocolConv mdbProtoConv;
    MDBProtocolTermiosDummy mdbProtoTermiosDummy;
    Linux::TermiosSerialDummy term;
    Linux::MillisTimer comTimer;

    comTimer.start();

    if (term.openComMDB(devPort, Linux::TermiosSerialDummy::BAUD_9600))
    {
        bv.initialize(&mdbProtoConv, &term, &comTimer);

        while (true)
        {
            bv.listen();
        }

        term.closeComMDB();
    }    

    debug << "Bye"
          << "\n";

    return 0;
}
