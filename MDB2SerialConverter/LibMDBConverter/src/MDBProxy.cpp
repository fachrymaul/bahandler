/**
 * @file MDBProxy.cpp
 * @brief Implementation of MDBProxy.hpp
 *
 * @copyright All Rights Reserved to Bukalapak
 * @author Bobby P. Johan
 * @author Andri Rahmadhani
 * @author Richard
 */

#include "MDBProxy.hpp"
#include <String.h>

static const uint8_t HEADER          = 0xFE;
static const uint8_t HEADER_LENGTH   = 2;
static const uint8_t SLAVE_TO_MASTER = 3;
static const uint8_t CRC_LENGTH      = 2;
static const uint8_t MSB_CHECKSUM    = 2;
static const uint8_t LSB_CHECKSUM    = 1;
static const uint8_t BIT_9_MASK      = 1;
static const uint8_t START_9_BIT     = 2;
static const uint8_t SLV_TO_MSTR_RES = 2;
static bool statusLed                = true;
static const uint8_t ACK = 0x00;

static uint8_t mToS[255]   = {0};  // master to slave
static uint16_t sToM[255]  = {0}; // slave to master
static uint8_t m_sToMCount = 0;

static uint16_t const reset[2]        = {0x130, 0x30};
static uint16_t const setup[2]        = {0x131, 0x31};
static uint16_t const securityLow[4]  = {0x132, 0x00, 0x00, 0x32};
static uint16_t const securityHigh[4] = {0x132, 0x00, 0xFF, 0x33};
static uint16_t const poll[2]         = {0x133, 0x33};
static uint16_t const escrowIn[3]     = {0x135, 0xFF, 0x34};
static uint16_t const escrowOut[3]    = {0x135, 0x00, 0x35};
static uint16_t const billStacked[2]  = {0x136, 0x36};
static uint16_t const expansion[3]    = {0x137, 0x00, 0x37};
static uint16_t const pollWithAck[3]  = {0x133, 0x33, 0x00};

static uint8_t setupValue[27]                 = {0};
static uint8_t valueBVFeature                 = 0;
static uint8_t valueCountryAndCurrencyCode[2] = {0};
static uint8_t valueBillScale[2]              = {0};
static uint8_t valueDecimalPlace              = 0;
static uint8_t valueStackerCap[2]             = {0};
static uint8_t valueBillSecurityLVL[2]        = {0};
static uint8_t valueEscrow                    = 0;
static uint8_t valueBillTypeSetup[16]         = {0};

static uint8_t billStackedValue[2]            = {0};
static uint8_t manufactorCode[3]              = {0};
static uint8_t pollValue[3]                   = {0};

static uint32_t tick     = 0;
static uint32_t prevTick = 0;

MDBProxy::MDBProxy()
{
    m_frameState  = FRAME_HEADER_CHECK;
    m_master      = nullptr;
    m_slave       = nullptr;
    m_waitExecute = false;
    m_billRouting = 0xFF;
    m_statusBV    = 0;
}

/**
 * function for enable bill Type and send command to Bill Acceptor
 * @param  btSwitch for bill type want be active
 */
void MDBProxy::enableBillType(const M2P::BillTypeSwitch &btSwitch)
{
    m_activeBillType |= btSwitch;
    setBillType(m_activeBillType);
}

void MDBProxy::lockExecute(uint32_t delayMs)
{
    uint32_t prevTickExecute;
    m_waitExecute   = true;
    prevTickExecute = tick;
    while(m_waitExecute && (tick - prevTickExecute) < delayMs);
}

bool MDBProxy::unlockExecute()
{
    m_waitExecute = false;
    return true;
}

bool MDBProxy::resetBillRouting()
{
    m_billRouting = 0xFF;
    return true;
}

/**
 * function for disable bill type and send command to bill acceptor
 * @param btSwitch for bill type want be deactive
 */
void MDBProxy::disableBillType(const M2P::BillTypeSwitch &btSwitch)
{
    m_activeBillType &= ~btSwitch;
    setBillType(m_activeBillType);
}

/**
 * send/transmit set of words to data register
 *
 * @param data array of word to be sent
 * @param lengthData length of data 
 * @param usart host address (master/slave)
 */
bool MDBProxy::sendDataUint8(uint8_t *data, const uint8_t lengthData, USART_TypeDef *usart)
{
    for (uint8_t i = 0; i < lengthData; i++)
    {
        while (!USART_GetFlagStatus(usart, USART_FLAG_TXE));
        usart->DR = data[i];
    }
    return true;
}
bool MDBProxy::sendDataUint8(uint8_t data, USART_TypeDef *usart)
{
    while (!USART_GetFlagStatus(usart, USART_FLAG_TXE));
    usart->DR = data;
    return true;
}

/**
 * Send data 9 bit 
 * @param data array of word to be sent
 * @param lengthData is length of Data
 * @param USART_TypeDef for choose usart use connect to master or slave
 */
bool MDBProxy::sendDataUint16(const uint16_t *data, const uint8_t lengthData, USART_TypeDef *usart)
{
    for (uint8_t i = 0; i < lengthData; i++)
    {
        while (!USART_GetFlagStatus(usart, USART_FLAG_TXE));
        usart->DR = data[i];
    }
    return true;
}
bool MDBProxy::sendDataUint16(uint16_t data, USART_TypeDef *usart)
{
    while (!USART_GetFlagStatus(usart, USART_FLAG_TXE));
    usart->DR = data;
    return true;
}

/**
 * Set Bill Type for bill acceptor
 * @param enable bill Type , for billtype we use
 */
void MDBProxy::setBillType(const uint8_t &billType)
{
    enum EnaBillTypeCmdIndex
    {
        I_BILL_TYPE = 2,
        I_ESCROW_IN = 4,
        I_CHECK_SUM = 5,
    };
    uint16_t enaBillCmdSet[6] = {0x134, 0x00, billType, 0x00, billType, 0x00};
    uint16_t calculateCheckSum = 0;

    for (uint8_t i = 0; i <= I_CHECK_SUM; i++)
    {
        if (i == I_CHECK_SUM)
        {
            enaBillCmdSet[I_CHECK_SUM] = (uint8_t)(calculateCheckSum & 0xFF);
        }
        else
        {
            calculateCheckSum += enaBillCmdSet[i];
        }
        sendDataUint16(enaBillCmdSet[i], m_slave);
    }
}

/**
 * sendCommand validate command and send command
 * @param data to use header,length and cmd , data, crc
 */
bool MDBProxy::sendCommand(uint8_t dataCommand)
{
    bool status  = true;
    uint8_t data = dataCommand;

    switch (m_frameState)
    {
    case FRAME_HEADER_CHECK:
        if (data == HEADER)
        {
            m_frameState = FRAME_CMD_CHECK;
            m_packet.checksum   = 0;
            m_packet.cmdId      = 0;
            m_packet.paramLen   = 0;
            m_packet.paramCount = 0;
            mToS[0] = data;
        }
        break;

    case FRAME_CMD_CHECK:
        m_packet.cmdId    = data      & 0x03;
        m_packet.paramLen = data >> 2 & 0x7F;

        switch (m_packet.cmdId)
        {
        case DIR_M2S_MSG:
            mToS[1] = data;
            m_frameState = FRAME_DATA;
            break;
        case DIR_M2S_RESP:
            m_frameState = FRAME_RESPONSE;
            break;
        default:
            m_frameState = HEADER;
            mToS[0] = 0;
            break;
        }
        break;

    case FRAME_DATA:
        mToS[m_packet.paramCount + HEADER_LENGTH] = data;
        m_packet.paramCount++;
        if (m_packet.paramCount >= m_packet.paramLen + CRC_LENGTH)
        {
            m_frameState = FRAME_CHECKSUM;
        }
        else break;

    case FRAME_CHECKSUM:
        if (m_crc16->crc16_ccitt((char *)mToS, m_packet.paramLen + HEADER_LENGTH) ==
            ((mToS[m_packet.paramLen + HEADER_LENGTH + CRC_LENGTH - MSB_CHECKSUM] << 8) | mToS[m_packet.paramLen + HEADER_LENGTH + CRC_LENGTH - LSB_CHECKSUM]))
        {
            status = sendApiCommand(mToS[2]);
            memset(mToS, 0x00, sizeof(mToS));
            m_frameState = FRAME_HEADER_CHECK;
        }
        else
        {
            memset(mToS, 0x00, sizeof(mToS));
            m_frameState = FRAME_HEADER_CHECK;
            status = false;
        }
        break;

    case FRAME_RESPONSE:
        sendDataUint16((uint16_t)(data & 0x01FF), m_slave);        
        status = true;
        m_frameState = FRAME_HEADER_CHECK;
        memset(mToS, 0x00, sizeof(mToS));
        break;
    }
    return status;
}

/**
 * sendApicommand, send data to slave(BV) , with many feature for this function
 * reset, setup, accept money, etc
 * @param data choose what feature wanna execute
 */
bool MDBProxy::sendApiCommand(uint8_t data)
{
    switch (data)
    {
    case M2P::CMD_RESET:
        return sendDataUint16(reset, sizeof(reset) / sizeof(reset[0]), m_slave);

    case M2P::CMD_ESCROW_OUT:
        return sendDataUint16(escrowOut, sizeof(escrowOut) / sizeof(escrowOut[0]), m_slave);

    case M2P::CMD_SETUP:
        m_stateActivity = STATE_SETUP;
        sendDataUint16(setup, sizeof(setup) / sizeof(setup[0]), m_slave);
        lockExecute(50);
        enableBillType(M2P::SW_BILL_ALL);
        return true;

    case M2P::CMD_BILL_ENBL_ALL:    enableBillType(M2P::SW_BILL_ALL);   return true;
    case M2P::CMD_BILL_ENBL_1K:     enableBillType(M2P::SW_BILL_1K);    return true;
    case M2P::CMD_BILL_ENBL_2K:     enableBillType(M2P::SW_BILL_2K);    return true;
    case M2P::CMD_BILL_ENBL_5K:     enableBillType(M2P::SW_BILL_5K);    return true;
    case M2P::CMD_BILL_ENBL_10K:    enableBillType(M2P::SW_BILL_10K);   return true;
    case M2P::CMD_BILL_ENBL_20K:    enableBillType(M2P::SW_BILL_20K);   return true;
    case M2P::CMD_BILL_ENBL_50K:    enableBillType(M2P::SW_BILL_50K);   return true;
    case M2P::CMD_BILL_ENBL_100K:   enableBillType(M2P::SW_BILL_100K);  return true;
     
    case M2P::CMD_BILL_DSBL_ALL:    disableBillType(M2P::SW_BILL_ALL);  return true;
    case M2P::CMD_BILL_DSBL_1K:     disableBillType(M2P::SW_BILL_1K);   return true;
    case M2P::CMD_BILL_DSBL_2K:     disableBillType(M2P::SW_BILL_2K);   return true;
    case M2P::CMD_BILL_DSBL_5K:     disableBillType(M2P::SW_BILL_5K);   return true;
    case M2P::CMD_BILL_DSBL_10K:    disableBillType(M2P::SW_BILL_10K);  return true;
    case M2P::CMD_BILL_DSBL_20K:    disableBillType(M2P::SW_BILL_20K);  return true;
    case M2P::CMD_BILL_DSBL_50K:    disableBillType(M2P::SW_BILL_50K);  return true;
    case M2P::CMD_BILL_DSBL_100K:   disableBillType(M2P::SW_BILL_100K); return true;

    case M2P::CMD_POLL:
        m_stateActivity = STATE_POLL;
        sendDataUint16(poll, sizeof(poll) / sizeof(poll[0]), m_slave);
        lockExecute(50);
        return true;

    case M2P::CMD_EXPANSION :
        m_stateActivity = STATE_EXPANSION;
        sendDataUint16(poll, sizeof(poll) / sizeof(poll[0]), m_slave);
        lockExecute(50);

    case M2P::CMD_ACCEPT_MONEY:
        m_stateActivity = STATE_POLL;
        sendDataUint16(poll, sizeof(poll) / sizeof(poll[0]), m_slave);
        lockExecute(50);

        if (m_billRouting == ESCROW_POSITION)
        {
            sendDataUint16(escrowIn, sizeof(escrowIn) / sizeof(escrowIn[0]), m_slave);
            resetBillRouting();
            prevTick = tick;
            while (m_billRouting != BILL_STACKED && tick - prevTick < 3000)
            {
                m_stateActivity = STATE_POLL;
                sendDataUint16(poll, sizeof(poll) / sizeof(poll[0]), m_slave);
                lockExecute(50);
            }
        }
        resetBillRouting();
        return true;

    case M2P::CMD_GET_INFO:
    
        m_stateActivity = STATE_EXPANSION;
        sendDataUint16(expansion, sizeof(expansion) / sizeof(expansion[0]), m_slave);
        lockExecute(50);

        m_stateActivity = STATE_SETUP;
        sendDataUint16(setup, sizeof(setup) / sizeof(setup[0]), m_slave);
        lockExecute(50);

        m_stateActivity = STATE_BILL_STACKED;
        sendDataUint16(billStacked, sizeof(billStacked) / sizeof(billStacked[0]), m_slave);
        lockExecute(50);

        sendDataUint8(manufactorCode, sizeof(manufactorCode), m_master);
        sendDataUint8(valueCountryAndCurrencyCode, sizeof(valueCountryAndCurrencyCode), m_master);
        sendDataUint8(m_activeBillType, m_master);
        sendDataUint8(billStackedValue, sizeof(billStackedValue), m_master);
        return sendDataUint8(m_statusBV, m_master);

    default:
        return false;
    }
}

/**
 * execute protocol slave to master
 * receive data from bill acceptor
 * @param data from bill acceptor
 */
bool MDBProxy::executeProtocolSTM(uint16_t dataSlave)
{
    sToM[m_sToMCount]   = dataSlave;
    if ((sToM[m_sToMCount] & 0x100) && m_sToMCount > 0)
    {
        m_sToMCount++;
        switch (m_stateActivity)
        {
        case STATE_DEFAULT:
            break;
        
        case STATE_RESET:
            break;

        case STATE_SETUP:
            for (uint8_t i = 0; i < m_sToMCount; i++)
            {
                setupValue[i] = sToM[i];
            }

            valueBVFeature    = setupValue[S_POS_BV_FEATURE];
            valueDecimalPlace = setupValue[S_POS_DECIMAL];
            valueEscrow       = setupValue[S_POS_ESCROW];
            for (uint8_t i = 0; i < 2; i++)
            {
                valueCountryAndCurrencyCode[i] = setupValue[i + S_POS_COUNTRY_AND_CURRENCY];
                valueStackerCap[i]             = setupValue[i + S_POS_STACKER];
                valueBillSecurityLVL[i]        = setupValue[i + S_POS_BILL_SECURITY];
                valueBillScale[i]              = setupValue[i + S_POS_BILL_SCALE];
            }
            for (uint8_t i = 0; i < 16; i++)
            {
                valueBillTypeSetup[i] = setupValue[i + S_POS_BILL_TYPE];
            }
            break;

        case STATE_SECURITY:
            break;

        case STATE_BILL_TYPE:
            break;
        
        case STATE_ESCROW:
            break;

        case STATE_BILL_STACKED:
            for (uint8_t i = 0; i < m_sToMCount; i++)
            {
                billStackedValue[i] = sToM[i];
            }
            break;
        
        case STATE_EXPANSION:
            for (uint8_t i = 0; i < m_sToMCount; i++)
            {
                manufactorCode[i] = sToM[i];
            }
            break;

        case STATE_POLL:
            for (uint8_t i = 0; i < m_sToMCount; i++)
            {
                pollValue[i] = sToM[i];
            }

            if((pollValue[0] & 0x80) == 0x80) // contains bill routing
            {
                m_billRouting = (pollValue[0] >> 4) & 0x07;
                uint8_t tempMoneyFromBV = pollValue[0] & 0x0f;
                switch (m_billRouting)
                {
                case BILL_STACKED:
                    sendDataUint8(ACK, m_slave);
                    sendDataUint8(tempMoneyFromBV, m_master);
                    break;

                case ESCROW_POSITION:
                    sendDataUint8(ACK, m_slave);
                    break;

                case BILL_RETURNED:
                    sendDataUint8(ACK, m_slave);
                    setBillType(m_activeBillType);
                    break;

                case BILL_TO_REYCYLER:
                    break;

                case DISABLED_BILL_REJECTED:
                    break;

                case BILL_TO_REYCYLER_MANUAL:
                    break;

                case MANUAL_DISPENSE:
                    break;

                case TRANSFERRED_FROM_REYCYLER:
                    break;

                default:
                    break;
                }
                m_statusBV = pollValue[1];
            }
            else // directly status BV without bill routing
            {
                m_statusBV = pollValue[0];           
            }

            switch (m_statusBV)
            {
            case STAT_BV_MOTOR_ERR:
                break;

            case STAT_BV_SENSOR_ERR:
                break;

            case STAT_BV_BUSY_ERR:
                sendDataUint8(ACK, m_slave);
                break;

            case STAT_BV_CHECKSUM_ERR:
                break;

            case STAT_BV_JAMMED_ERR:
                break;

            case STAT_BV_WAS_RESET:
                sendDataUint8(ACK, m_slave);
                break;

            case STAT_BV_BILL_REMOVED:
                break;

            case STAT_BV_BOX_OUT_POSITION:
                break;

            case STAT_BV_DISABLE:
                sendDataUint8(ACK, m_slave);
                break;

            case STAT_BV_INV_ESCROW_REQ:
                sendDataUint8(ACK, m_slave);
                break;

            case STAT_BV_BILL_REJECTED:
                sendDataUint8(ACK, m_slave);
                break;

            case STAT_BV_POS_BILL_REMOVED:
                sendDataUint8(ACK, m_slave);
                break;
            }
            break; // m_stateActivity case STATE_POLL

        default:
            break;
        }
        memset(sToM, 0x00, sizeof(sToM) / sizeof(sToM[0]));
        m_sToMCount = 0;
        m_stateActivity = STATE_DEFAULT;
        unlockExecute();
        if (statusLed)
        {
            statusLed = !statusLed;
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        }
        else
        {
            statusLed = !statusLed;
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
        }
        return true;
    }

    else if ((sToM[m_sToMCount] & 0x100) == 0x100 && m_sToMCount < 1)
    {
        m_statusBV = ACK;
        unlockExecute();
        if (statusLed)
        {
            statusLed = !statusLed;
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        }
        else
        {
            statusLed = !statusLed;
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
        }
        return true;
    }
    m_sToMCount++;
    return true;
}

void MDBProxy::init(USART_TypeDef *master, USART_TypeDef *slave, CRC16 *crc16)
{
    m_master = master;
    m_slave  = slave;
    m_crc16  = crc16;
}

extern "C"
{
    void SysTick_Handler(void)
    {
        tick++;
    }
}
