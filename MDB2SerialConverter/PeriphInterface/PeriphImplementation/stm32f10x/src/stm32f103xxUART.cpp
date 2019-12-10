#include <stm32f103xxUART.hpp>
#include <stm32f103xxUARTInterrupt.hpp>

using namespace stm32f103xx;

Uart::Uart()
{
    m_USARTx = NULL;
    m_uartPort = 0;
    m_errCode = 0;
    m_txBuffer = NULL;
    m_txBufferLen = 0;
}

Uart::~Uart()
{
}

void Uart::initialize(USART_TypeDef* USARTx, uint8_t* txAsyncBuffer, uint32_t txBufferLen)
{
    m_USARTx = USARTx;
    m_uartPort = UartControl::lookupPortNum(USARTx); 
    m_txBuffer = txAsyncBuffer;
    m_txBufferLen = txBufferLen;
    m_errCode = 0;
}

Uart::CmdStatus Uart::transmit(uint8_t* txData, uint32_t txDataLen)
{
    if(!UartControl::lockTx(m_uartPort))
    {
        return CMD_UART_BUSY;
    }

    for(uint8_t idx = 0; idx < txDataLen; idx++)
    {
        //blocking until buffer ready
    	while(!(m_USARTx->SR & USART_FLAG_TXE));
    	m_USARTx->DR = txData[idx];
    }

    //blocking until transmission is complete
    while(!(m_USARTx->SR & USART_FLAG_TC));
    UartControl::unlockTx(m_uartPort);

    return CMD_SUCCESS;
}

Uart::CmdStatus Uart::receive(uint8_t* rxBuffer, uint32_t rxBufferSize, uint32_t* pRxDataLen)
{
    if(!UartControl::lockRx(m_uartPort))
    {
        return CMD_UART_BUSY;
    }

    if((m_USARTx->SR & 0x0F) > 0)
    {
        m_errCode |= m_USARTx->SR & 0x0F;
        UartControl::unlockRx(m_uartPort);
        return CMD_UART_ERROR;
    }
    
    *pRxDataLen = (m_USARTx->SR & USART_FLAG_RXNE) >> 0x05;
    if(*pRxDataLen)
    {
        rxBuffer[0] = m_USARTx->DR & 0xFF;
    }
    
    UartControl::unlockRx(m_uartPort);

    return CMD_SUCCESS;
}

uint32_t Uart::getDataCount()
{
    return (m_USARTx->SR & USART_FLAG_RXNE) >> 0x05;
}

uint16_t Uart::getErrorCode()
{
    uint8_t tempErrorCode = m_errCode;
    m_errCode = 0;

    if(tempErrorCode > 0)
    {
        //dummy read to clear SR error flag
        m_USARTx->DR;
    }

    return tempErrorCode;
}

Uart::CmdStatus Uart::transmitAsync(uint8_t* txData, uint32_t txDataLen, TxCallbackConf* pFinishedCbConf)
{
    if(txDataLen > m_txBufferLen)
    {
        m_errCode |= ERR_TX_BUFF_OVERFLOW_RISK;
        return CMD_UART_ERROR;
    }

    if(!UartControl::lockTx(m_uartPort))
    {
        return CMD_UART_BUSY;
    }

    memcpy(m_txBuffer, txData, sizeof(uint8_t) * txDataLen);
    
    if(!UartControl::enableTxCallback(m_uartPort, m_txBuffer, txDataLen, pFinishedCbConf))
    {
        m_errCode |= ERR_TX_ASYNC_INIT;
        UartControl::unlockTx(m_uartPort);
        return CMD_UART_ERROR;
    }

    return CMD_SUCCESS;
}

Uart::CmdStatus Uart::startReceiveAsync(RxCallbackConf* pReceivedCbConf)
{
    if(!UartControl::lockRx(m_uartPort))
    {
        return CMD_UART_BUSY;
    }
 
    if(!UartControl::enableRxCallback(m_uartPort, pReceivedCbConf))
    {
        m_errCode |= ERR_RX_ASYNC_INIT;
        UartControl::unlockRx(m_uartPort);
        return CMD_UART_ERROR;
    }

    return CMD_SUCCESS;
}

Uart::CmdStatus Uart::stopReceiveAsync()
{
    UartControl::disableRxCallback(m_uartPort);
    return CMD_SUCCESS;
}
