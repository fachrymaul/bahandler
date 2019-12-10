#include <stm32f103xxIRQPriorityConf.hpp>
#include <stm32f103xxUARTInterrupt.hpp>

namespace stm32f103xx
{
namespace UartControl
{
/********************* Private Type & Function Declaration ***************/
enum IRQStatus
{
    IRQ_DISABLED,
    IRQ_INITIALIZING,
    IRQ_ENABLED
};

struct IRQControl
{
    USART_TypeDef* USARTx;
    IRQStatus txIrqStatus;
    IRQStatus rxIrqStatus;
    bool nvicEnabled;
    IRQn usartIRQn;
};

struct UartTxDataQueue
{
    uint8_t* data;
    uint32_t len;
};

static void activateNVIC(uint8_t port);
static void disableNVIC(uint8_t port);

/*********************** Local Variables **************************/
static const uint8_t PORT_COUNT = 6;

static IUart::TxCallbackConf txCbConf[PORT_COUNT] = {{0}};
static IUart::RxCallbackConf rxCbConf[PORT_COUNT] = {{0}};

static volatile UartTxDataQueue txDataQueue[PORT_COUNT] = {0};
static uint8_t rxBuffer[PORT_COUNT] = {0};

static volatile bool txLock[PORT_COUNT] = {0};
static volatile bool rxLock[PORT_COUNT] = {0};

static IRQControl irqCtrl[PORT_COUNT] =
{
    // TODO: Implement for F103
    {USART1, IRQ_DISABLED, IRQ_DISABLED, false, USART1_IRQn},
    {USART2, IRQ_DISABLED, IRQ_DISABLED, false, USART2_IRQn},
    {USART3, IRQ_DISABLED, IRQ_DISABLED, false, USART3_IRQn}
};


/*********************** Public Functions **************************/
bool enableTxCallback(uint8_t port, uint8_t* txDataBuffer, uint32_t txBufferLen, IUart::TxCallbackConf* pCbConf)
{
    //these two ifs used to prevent compiler reordering
    if(irqCtrl[port].txIrqStatus  == IRQ_DISABLED)
    {
        txDataQueue[port].data = txDataBuffer;
        txDataQueue[port].len = txBufferLen;
        txCbConf[port] = *pCbConf;
        irqCtrl[port].txIrqStatus = IRQ_INITIALIZING;
    }

    if(irqCtrl[port].txIrqStatus == IRQ_INITIALIZING)
    {
        //enable tx interrupt
        irqCtrl[port].USARTx->CR1 |= 0x01 << 7;
        activateNVIC(port);
        irqCtrl[port].txIrqStatus = IRQ_ENABLED;
        return true;
    }

    //if this is reached, reordering somehow is occured
    return false;
}

bool enableRxCallback(uint8_t port, IUart::RxCallbackConf* pCbConf)
{
    if(irqCtrl[port].rxIrqStatus == IRQ_DISABLED)
    {
        rxCbConf[port] = *pCbConf;
        irqCtrl[port].rxIrqStatus = IRQ_INITIALIZING;
    }

    if(irqCtrl[port].rxIrqStatus == IRQ_INITIALIZING)
    {
        //enable receive and parity error interrupt
        irqCtrl[port].USARTx->CR1 |= ((0x01 << 5) | (0x01 << 8));
        activateNVIC(port);
        irqCtrl[port].rxIrqStatus = IRQ_ENABLED;
        return true;
    }

    //if this is reached, reordering somehow is occured
    return false;
}

void disableTxCallback(uint8_t port)
{
    //disable tx interrupt
    irqCtrl[port].USARTx->CR1 &= ~(0x01 << 7);

    if(irqCtrl[port].rxIrqStatus == IRQ_DISABLED)
    {
        disableNVIC(port);
    }

    irqCtrl[port].txIrqStatus = IRQ_DISABLED;
    unlockTx(port);
}

void disableRxCallback(uint8_t port)
{
    //disable rx interrupt
    irqCtrl[port].USARTx->CR1 &= ~((0x01 << 5) | (0x01 << 8));

    if(irqCtrl[port].txIrqStatus == IRQ_DISABLED)
    {
        disableNVIC(port);
    }

    irqCtrl[port].rxIrqStatus = IRQ_DISABLED;
    unlockRx(port);
}

bool lockTx(uint8_t port)
{
    //TODO : implement atomic/thread-safe lock
    if(txLock[port])
    {
        return false;
    }

    txLock[port] = true;
    return true;
}

void unlockTx(uint8_t port)
{
    txLock[port] = false; 
}


bool lockRx(uint8_t port)
{
    //TODO : implement atomic/thread-safe lock
    if(rxLock[port])
    {
        return false;
    }

    rxLock[port] = true;
    return true;
}

void unlockRx(uint8_t port)
{
    rxLock[port] = false; 
}

uint8_t lookupPortNum(USART_TypeDef* USARTx)
{
    // TODO: Implement for F103
    if(USARTx == USART1) return 0;
    if(USARTx == USART2) return 1;
    if(USARTx == USART3) return 2;

    return 0;
}
/*********************** Private Functions **************************/

static void activateNVIC(uint8_t port)
{
    if(irqCtrl[port].nvicEnabled)
    {
        return;
    }

    NVIC_SetPriority(irqCtrl[port].usartIRQn, USART_IRQN_PRIORITY);
    NVIC_EnableIRQ(irqCtrl[port].usartIRQn);

    irqCtrl[port].nvicEnabled = true;
}

static void disableNVIC(uint8_t port)
{
    if(!(irqCtrl[port].nvicEnabled))
    {
        return;
    }

    NVIC_DisableIRQ(irqCtrl[port].usartIRQn);

    irqCtrl[port].nvicEnabled = false;
}

static void USART_IRQProcess(uint8_t port)
{
    //error interrupt
    if(irqCtrl[port].USARTx->SR & 0x0F)
    {
        if(rxCbConf[port].cb) rxCbConf[port].cb(true, &rxBuffer[port], 0, NULL);
    }

    //rx data ready
    if(irqCtrl[port].USARTx->SR & USART_FLAG_RXNE)
    {
        rxBuffer[port] = irqCtrl[port].USARTx->DR;
        if(rxCbConf[port].cb) rxCbConf[port].cb(false, &rxBuffer[port], 1, rxCbConf[port].cbArg);
    }

    //tx interrupt
    if(irqCtrl[port].USARTx->SR & USART_FLAG_TXE)
    {
        if(txDataQueue[port].len > 0)
        {
            irqCtrl[port].USARTx->DR = *(txDataQueue[port].data++);
            txDataQueue[port].len--;
        }
        else if(irqCtrl[port].USARTx->SR & USART_FLAG_TC)
        {
            disableTxCallback(port);
            if(txCbConf[port].cb) txCbConf[port].cb(false, txCbConf[port].cbArg);
        }
    }
}
}
}

/************************* Interrupt Handlers ***********************/
extern "C"
{
void USART1_IRQHandler()
{
    stm32f103xx::UartControl::USART_IRQProcess(0);
}

void USART2_IRQHandler()
{
    stm32f103xx::UartControl::USART_IRQProcess(1);
}

void USART3_IRQHandler()
{
    stm32f103xx::UartControl::USART_IRQProcess(2);
}

void UART4_IRQHandler()
{
    stm32f103xx::UartControl::USART_IRQProcess(3);
}

void UART5_IRQHandler()
{
    stm32f103xx::UartControl::USART_IRQProcess(4);
}

void USART6_IRQHandler()
{
    stm32f103xx::UartControl::USART_IRQProcess(5);
}

}