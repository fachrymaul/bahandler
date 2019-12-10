#ifndef STM32F103XX_UART_H
#define STM32F103XX_UART_H

#include <stm32f10x.h>
#include <IPeriphInterface.hpp>

namespace stm32f103xx
{
class Uart final : public IUart
{
public:
    enum ErrorCodeMask
    {
        ERR_RX_PARITY = 1,
        ERR_RX_FRAMING = 2,
        ERR_RX_NOISE = 4,
        ERR_RX_OVERRUN = 8,
        ERR_TX_BUFF_OVERFLOW_RISK = 16,
        ERR_TX_ASYNC_INIT = 32,
        ERR_RX_ASYNC_INIT = 64
    };

    Uart();
    ~Uart();
    void initialize(USART_TypeDef* USARTx, uint8_t* txAsyncBuffer, uint32_t txBufferLen);

    virtual CmdStatus transmit(uint8_t* txData, uint32_t txDataLen);
    virtual CmdStatus receive(uint8_t* rxBuffer, uint32_t rxBufferSize, uint32_t* pRxDataLen);
    virtual uint32_t getDataCount();
    virtual uint16_t getErrorCode();

    virtual CmdStatus transmitAsync(uint8_t* txData, uint32_t txDataLen, TxCallbackConf* pFinishedCbConf);
    virtual CmdStatus startReceiveAsync(RxCallbackConf* pReceivedCbConf);
    virtual CmdStatus stopReceiveAsync();

private:
    USART_TypeDef* m_USARTx;
    uint8_t m_uartPort;
    uint8_t m_errCode;
    uint8_t* m_txBuffer;
    uint32_t m_txBufferLen;
};
}

#endif