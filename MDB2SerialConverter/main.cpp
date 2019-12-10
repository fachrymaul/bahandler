#include <stm32f10x.h>
#include <stm32f10x_conf.h>
#include "MDBProxy.hpp"

MDBProxy mdb;
CRC16 crc16;

const uint8_t size = 255;
volatile uint8_t dataFromMaster[size] = {0};
volatile uint32_t positionDataFromMasterHead = 0;
volatile uint32_t positionDataFromMasterTail = 0;
bool overFlowStatus = true;

static void cmdUartCallBackSTM(uint16_t data)
{
    mdb.executeProtocolSTM(data);
}

void initLed()
{
    /* init led untuk mempermudah melihat apa ada data yang masuk */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void Config_Systick()
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency/1000); // SysTick interrupt set 1ms
}

void initUSART1()
{
    /*Initialize Pin Gpio to USART1*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure USART1_RX as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure USART1 parameters */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_InitTypeDef usart;
    usart.USART_BaudRate = 9600;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_WordLength = USART_WordLength_8b;
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
}

void initUSART2()
{
    /*Set Pin Gpio to USART2*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure USART1_RX as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART2 parameters */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 19200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

void initUSART3()
{
    /*Set Pin Gpio to USART3*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure USART3 parameters */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_InitTypeDef usart;
    usart.USART_BaudRate = 9600;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_WordLength = USART_WordLength_9b;
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Init(USART3, &usart);
    USART_Cmd(USART3, ENABLE);
}

void initNvic()
{
    /* Active Interupt for USART2 */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStruct);

    /* Active Interupt for USART1 */
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);

    /* Active Interupt for USART3 */
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);
}

void initIWDG()
{
    /* Active Watch Dog and Set Config watch dog */
    IWDG_WriteAccessCmd(0x5555);
    IWDG_SetPrescaler(IWDG_Prescaler_64);
    IWDG_SetReload(2559); //five sec timeout
    IWDG_ReloadCounter();
    IWDG_Enable();
}

int main()
{   
    SystemInit();
    initUSART2();
    initUSART3();
    initIWDG();
    initNvic();
    initLed();
    SysTick_Config(SystemCoreClock/1000);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    mdb.init(USART2, USART3, &crc16); //master slave
    mdb.sendApiCommand(M2P::CMD_RESET);
    mdb.lockExecute(1000);
    mdb.sendApiCommand(M2P::CMD_SETUP);
    mdb.lockExecute(1000);
    mdb.sendApiCommand(M2P::CMD_EXPANSION);
    mdb.lockExecute(100);
    
    while (1)
    {
        mdb.sendApiCommand(M2P::CMD_POLL);
        if(positionDataFromMasterHead > positionDataFromMasterTail)
        {
            mdb.sendCommand(dataFromMaster[positionDataFromMasterTail]);
            positionDataFromMasterTail++;
        }
        else
        {
             if(overFlowStatus)
             {
                mdb.sendCommand(dataFromMaster[positionDataFromMasterTail]);   
                positionDataFromMasterTail++;
                if(positionDataFromMasterTail > size - 1)
                {
                    positionDataFromMasterTail = 0;
                    overFlowStatus = false;
                }
             }
        }
        IWDG_ReloadCounter();   
    }
}

extern "C"
{
    void USART1_IRQHandler()
    {
        if (USART_GetITStatus(USART1, USART_IT_RXNE))
        {
         
        }
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
    
    void USART2_IRQHandler() //master
    {
        if (USART_GetITStatus(USART2, USART_IT_RXNE))
        {
            // cmdUartCallBackMTS(USART2->DR);
            dataFromMaster[positionDataFromMasterHead] = USART2->DR;
            positionDataFromMasterHead++;
            if(positionDataFromMasterHead > size - 1)
            {
                positionDataFromMasterHead = 0;
                overFlowStatus = true;
            }
        }
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
    }
    
    void USART3_IRQHandler() //slave
    {
        if (USART_GetITStatus(USART3, USART_IT_RXNE))
        {
           cmdUartCallBackSTM(USART3->DR);
        }
        USART_ClearITPendingBit(USART3,USART_IT_RXNE);
    }
}
