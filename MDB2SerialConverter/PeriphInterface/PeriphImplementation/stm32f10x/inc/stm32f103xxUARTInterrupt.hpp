#ifndef STM32F103XX_UART_INTERRUPT_H
#define STM32F103XX_UART_INTERRUPT_H

#include <IPeriphInterface.hpp>
#include <stm32f10x.h>

namespace stm32f103xx
{
namespace UartControl
{
bool enableTxCallback(uint8_t port, uint8_t* txDataBuffer, uint32_t txBufferLen, IUart::TxCallbackConf* pCbConf);
void disableTxCallback(uint8_t port);

bool enableRxCallback(uint8_t port, IUart::RxCallbackConf* pCbConf);
void disableRxCallback(uint8_t port);

bool lockTx(uint8_t port);
void unlockTx(uint8_t port);

bool lockRx(uint8_t port);
void unlockRx(uint8_t port);

uint8_t lookupPortNum(USART_TypeDef* USARTx);
}
}

extern "C" 
{
void USART1_IRQHandler();
void USART2_IRQHandler();
void USART3_IRQHandler();
}

#endif