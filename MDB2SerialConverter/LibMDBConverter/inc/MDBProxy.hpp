/**
 * @file MDBProxy.hpp
 * @brief Implementation of MDBProxy.hpp
 *
 * @copyright All Rights Reserved to Bukalapak
 * @author Bobby P. Johan
 * @author Andri Rahmadhani
 * @author Richard
 */
#ifndef MDB_PROXY_H
#define MDB_PROXY_H

#include "stm32f10x_usart.h"
#include "CRC16.hpp"
#include "Master2Proxy.hpp"

class MDBProxy
{
public:
    MDBProxy();
    void init(USART_TypeDef *Master, USART_TypeDef *Slave, CRC16 *crc16);
    /**
     * Check data from master header , cmdId , data and checksum
     * and Call api command for send Data
     * 
     * @param dataCommand Header ,cmdId, data, checksum
     */
    bool apiCommand(uint8_t command);
    
    /**
     * sendCommand validate command and send command
     * @param data to use header,length and cmd , data, crc
     */
    bool sendCommand(uint8_t dataCommand);

    /**
     * function delay to wait interupt execute data and lock until finish
     * @param delayMs for timeout lock
     */
    void lockExecute(uint32_t delayMs);
    
    /**
     * sendApicommand, send data to slave(BV) , with many feature for this function
     * reset, setup, accept money, etc
     * @param data choose what feature wanna execute
     */
    bool sendApiCommand(uint8_t data);

    /**
     * execute protocol slave to master
     * receive data from bill acceptor and send data to master
     * @param data from bill acceptor
     */
    bool executeProtocolSTM(uint16_t dataSlave);

private:
    enum PacketDirection
    {
        DIR_M2S_MSG = 0,
        DIR_M2S_RESP,
        DIR_S2M_MSG,
        DIR_S2M_RESP
    };

    enum Setup
    {
        S_POS_BV_FEATURE           =  0,
        S_POS_COUNTRY_AND_CURRENCY =  1,
        S_POS_BILL_SCALE           =  3,
        S_POS_DECIMAL              =  5,
        S_POS_STACKER              =  6,
        S_POS_BILL_SECURITY        =  8,
        S_POS_ESCROW               = 10,
        S_POS_BILL_TYPE            = 11,
    };

    enum FrameState
    {
        FRAME_HEADER_CHECK = 1,
        FRAME_CMD_CHECK,
        FRAME_LENGTH_CHECK,
        FRAME_DATA,
        FRAME_CHECKSUM,
        FRAME_RESPONSE
    };

    enum StateActivity
    {
        STATE_DEFAULT = 0,
        STATE_RESET,
        STATE_SETUP,
        STATE_SECURITY,
        STATE_BILL_TYPE,
        STATE_ESCROW,
        STATE_BILL_STACKED,
        STATE_EXPANSION,
        STATE_POLL,
        STATE_POLL_READ_STATUS
    };

    enum BillRouting
    {
        BILL_STACKED = 0,
        ESCROW_POSITION,
        BILL_RETURNED,
        BILL_TO_REYCYLER,
        DISABLED_BILL_REJECTED,
        BILL_TO_REYCYLER_MANUAL,
        MANUAL_DISPENSE,
        TRANSFERRED_FROM_REYCYLER
    };

    enum StatusBV
    {
        STAT_BV_MOTOR_ERR = 1,
        STAT_BV_SENSOR_ERR,
        STAT_BV_BUSY_ERR,
        STAT_BV_CHECKSUM_ERR,
        STAT_BV_JAMMED_ERR,
        STAT_BV_WAS_RESET,
        STAT_BV_BILL_REMOVED,
        STAT_BV_BOX_OUT_POSITION,
        STAT_BV_DISABLE,
        STAT_BV_INV_ESCROW_REQ,
        STAT_BV_BILL_REJECTED,
        STAT_BV_POS_BILL_REMOVED,
    };

    struct CommandPacket
    {
        uint8_t  cmdId;
        uint8_t  paramLen;
        uint8_t  paramCount;
        uint16_t checksum;
        uint8_t  paramChecksum;
    };

    USART_TypeDef *m_master;
    USART_TypeDef *m_slave;
    uint8_t       m_frameState;
    CommandPacket m_packet;
    CRC16         *m_crc16;
    StateActivity m_stateActivity;
    uint8_t       m_activeBillType = M2P::SW_BILL_NONE;
    bool          m_waitExecute;
    uint8_t       m_billRouting;
    uint8_t       m_statusBV;

    /**
     * send/transmit set of words to data register
     *
     * @param data array of word to be sent
     * @param lengthData length of data 
     * @param usart host address (master/slave)
     */
    bool sendDataUint8(uint8_t *data, const uint8_t lengthData, USART_TypeDef *usart);

    /**
     * Send data 9 bit 
     * @param data array of word to be sent
     * @param lengthData is length of Data
     * @param USART_TypeDef for choose usart use connect to master or slave
     */
    bool sendDataUint16(const uint16_t *data, const uint8_t lengthData, USART_TypeDef *usart);

    /**
     * function for enable bill Type and send command to Bill Acceptor
     * @param btSwitch particular bill type to be activated
     */
    void enableBillType(const M2P::BillTypeSwitch &btSwitch);

    /**
     * function for disable bill type and send command to bill acceptor
     * @param btSwitch particular bill type to be deactivated
     */
    void disableBillType(const M2P::BillTypeSwitch &btSwitch);

    /**
     * Set Bill Type for bill acceptor
     * @param billType to be set as active bill type switch
     */
    void setBillType(const uint8_t &billType);

    /**
     * send 1 byte data
     * @param data data to be sent
     * @param *usart serial host address: from usart 1,2 or 3
     */
    bool sendDataUint8(uint8_t data, USART_TypeDef *usart);

    /**
     * send 9 bits data
     * @param data data to be sent
     * @param *usart serial host address: from usart 1,2 or 3
     */
    bool sendDataUint16(uint16_t data, USART_TypeDef *usart);

    /**
     * unlock flag execute data
     */
    bool unlockExecute();

    /**
     * reset billrouting to 0xFF 
     */
    bool resetBillRouting();
};
#endif
