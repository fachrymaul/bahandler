#ifndef MDB_PROTOCOL_TERMIOS_DUMMY_H
#define MDB_PROTOCOL_TERMIOS_DUMMY_H

#include <MDBProtocolBase.hpp>

class MDBProtocolTermiosUSB final : public MDBProtocolBase
{
  private:
    const uint16_t MDB_TERMIOS_MODEBIT_HDR = 0xFF00;

  public:
    MDBProtocolTermiosUSB();
    ~MDBProtocolTermiosUSB();

    virtual CmdStatus parseRecvDataMaster(uint8_t *inData, uint8_t inDataLen,
                                          uint8_t *outData, uint8_t *outDataLen) override;
    virtual CmdStatus parseRecvDataSlave(uint8_t *inData, uint8_t inDataLen,
                                         uint8_t *addr, uint8_t *cmd, int16_t *subCmd,
                                         uint8_t *outData, uint8_t *outDataLen) override;
};

#endif
