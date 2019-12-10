#ifndef SERIAL_H
#define SERIAL_H

#include "../IPeriph.hpp"
#include <termios.h>

class Serial : IUart
{
  public:
    int initialize(const char *PORT);
    CmdStatus transmit(uint8_t *txData, uint32_t txDataLen);
    CmdStatus receive(uint8_t *rxBuffer, uint32_t rxBufferSize, uint32_t *pRxDataLen); //specify the fifo size in init
    void listen();
    uint32_t getDataCount();
    uint16_t getErrorCode();

    void closePort();

    CmdStatus transmitAsync(uint8_t *txData, uint32_t txDataLen, TxCallbackConf *pFinishedCbConf);
    CmdStatus startReceiveAsync(RxCallbackConf *pReceivedCbConf);
    CmdStatus stopReceiveAsync();
  private:
    int m_serialPort = 0;
};

#endif // !SERIAL_H
