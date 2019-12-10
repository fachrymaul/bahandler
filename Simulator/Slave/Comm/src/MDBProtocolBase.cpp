#include <MDBProtocolBase.hpp>
#include <cstring>

MDBProtocolBase::MDBProtocolBase()
{
    m_pCom = nullptr;
    m_pTimer = nullptr;
    std::memset(m_txBuffer, 0x00, sizeof(m_txBuffer));
    std::memset(m_rxBuffer, 0x00, sizeof(m_rxBuffer));
    m_rxBufferLen = 0;
}

MDBProtocolBase::~MDBProtocolBase()
{
}

void MDBProtocolBase::initialize(ISerial *pCom, ITimer *pTimer)
{
    m_pCom = pCom;
    m_pTimer = pTimer;
}

MDBProtocolBase::CmdStatus MDBProtocolBase::sendCommandFromMaster(uint8_t address, uint8_t cmd,
                                                                  uint8_t *data, uint8_t dataLen)
{
    return sendCommandFromMaster(address, cmd, -1, data, dataLen);
}

MDBProtocolBase::CmdStatus MDBProtocolBase::sendCommandFromMaster(uint8_t address, uint8_t cmd,
                                                                  int16_t subCmd, uint8_t *data, 
                                                                  uint8_t dataLen)
{
    uint8_t sum = 0, idx = 0;

    // address and command
    m_txBuffer[idx++] = address | cmd;
    sum += address | cmd;

    // subcommand
    if (subCmd >= 0)
    {
        m_txBuffer[idx++] = (uint8_t)subCmd;
        sum += (uint8_t)subCmd;
    }

    // data
    for (int i = 0; i < dataLen; i++)
    {
        m_txBuffer[idx++] = data[i];
        sum += data[i];
    }

    // checksum
    m_txBuffer[idx++] = sum;

    return transmitPacket(m_txBuffer, idx, MDB_PROTO_MASTER,
                          MDB_MSG_CMD);
}

MDBProtocolBase::CmdStatus MDBProtocolBase::sendResponseFromMaster(MDBProtocolBase::CmdStatus resp)
{
    m_txBuffer[0] = (uint8_t)resp;

    return transmitPacket(m_txBuffer, 1, MDB_PROTO_MASTER,
                          MDB_MSG_RESP);
}

MDBProtocolBase::CmdStatus MDBProtocolBase::getResponseFromSlave(uint8_t *data, uint8_t *dataLen,
                                                                 uint8_t expectedLen, uint32_t timeoutMs)
{
    MDBProtocolBase::CmdStatus status = collectData(expectedLen, timeoutMs);

    if (status == CMD_STAT_OK)
    {
        status = parseRecvDataMaster(m_rxBuffer, m_rxBufferLen, data, dataLen);
    }
    return status;
}

MDBProtocolBase::CmdStatus MDBProtocolBase::sendDataFromSlave(uint8_t *data, uint8_t dataLen)
{
    uint8_t sum = 0, idx = 0;

    // data
    for (int i = 0; i < dataLen; i++)
    {
        m_txBuffer[idx++] = data[i];
        sum += data[i];
    }

    // checksum
    m_txBuffer[idx++] = sum;

    return transmitPacket(m_txBuffer, idx, MDB_PROTO_SLAVE, MDB_MSG_CMD);
}

MDBProtocolBase::CmdStatus MDBProtocolBase::sendResponseFromSlave(MDBProtocolBase::CmdStatus resp)
{
    m_txBuffer[0] = (uint8_t)resp;

    return transmitPacket(m_txBuffer, 1, MDB_PROTO_SLAVE,
                          MDB_MSG_RESP);
}

MDBProtocolBase::CmdStatus MDBProtocolBase::getResponseFromMaster(uint8_t *data, uint8_t *dataLen,
                                                                  uint8_t *addr, uint8_t *cmd, 
                                                                  int16_t *subCmd, uint8_t expectedLen, 
                                                                  uint32_t timeoutMs)
{
    return getDataFromMaster(data, dataLen, addr, cmd, subCmd, 1, timeoutMs);
}

MDBProtocolBase::CmdStatus MDBProtocolBase::getDataFromMaster(uint8_t *data, uint8_t *dataLen,
                                                              uint8_t *addr, uint8_t *cmd, 
                                                              int16_t *subCmd, uint8_t expectedLen,
                                                              uint32_t timeoutMs)
{
    MDBProtocolBase::CmdStatus status = collectData(expectedLen, timeoutMs);

    if (status == CMD_STAT_OK)
    {
        status = parseRecvDataSlave(m_rxBuffer, m_rxBufferLen, addr,
                                    cmd, subCmd, data, dataLen);
    }
    return status;
}

MDBProtocolBase::CmdStatus MDBProtocolBase::transmitPacket(uint8_t *data, uint8_t dataLen,
                                                           ProtocolMode mode, MessageType mType)
{
    if (!m_mutex.try_lock())
    {
        return CMD_STAT_COM_BUSY;
    }

    int8_t status = -1;

    if (mode == MDB_PROTO_MASTER)
    {
        if (mType == MDB_MSG_CMD)
        {
            // address byte and cmd
            m_pCom->setModeBit(true);
            status = m_pCom->queue((uint8_t *)&data[0], 1);

            // data (if any), exclude checksum
            if (dataLen > 2)
            {
                m_pCom->setModeBit(false);
                status = m_pCom->queue((uint8_t *)&data[1], dataLen - 1);
            }

            // start transmission
            status = m_pCom->transmitQueue();
        }
        else if (mType == MDB_MSG_RESP)
        {
            // response
            m_pCom->setModeBit(false);
            status = m_pCom->transmit((uint8_t *)&data[0], 1);
        }
    }
    else if (mode == MDB_PROTO_SLAVE)
    {
        if (mType == MDB_MSG_CMD)
        {
            if (dataLen > 1)
            {
                m_pCom->setModeBit(false);
                status = m_pCom->queue((uint8_t *)&data[0], dataLen - 1);

                // last byte, enable mode bit
                m_pCom->setModeBit(true);
                status = m_pCom->queue((uint8_t *)&data[dataLen - 1], 1);

                // start transmission
                status = m_pCom->transmitQueue();
            }
        }
        else if (mType == MDB_MSG_RESP)
        {
            // response only
            m_pCom->setModeBit(true);
            status = m_pCom->transmit((uint8_t *)&data[0], 1);
        }
    }

    if (status < 1)
        return CMD_STAT_COM_ERROR;

    m_mutex.unlock();

    return CMD_STAT_OK;
}

MDBProtocolBase::CmdStatus MDBProtocolBase::collectData(uint8_t expectedLen, uint32_t timeoutMs)
{
    uint64_t startMillis = m_pTimer->getMillis();
    // uint64_t startMillisIByte = startMillis;
    int16_t idx = 0;

    /*
    while(m_pTimer->getMillis() - startMillis < timeoutMs)
    {
        if(idx < MDB_MAX_RX_BUFFER_LEN)
        {
            int16_t readStat = m_pCom->receive(&m_rxBuffer[idx++], 1);
            if(readStat <= 0)
            {   
                // If current interbyte exceed max interbytetime, end of transmission
                if(m_pTimer->getMillis() - startMillisIByte > MDB_MAX_IBYTE_TIME)
                {
                    break;
                }
            }
            else
            {
                startMillisIByte = m_pTimer->getMillis();
            }
        }
    }
    */

    while (m_pTimer->getMillis() - startMillis < timeoutMs)
    {
        // Add buffer
        idx = m_pCom->receive(m_rxBuffer, expectedLen + MDB_SAFETY_BUFFER_LEN);
        if (idx == -1)
        {
            return CMD_STAT_COM_ERROR;
        }
        else if (idx > 0)
        {
            break;
        }
    }

    // No data received, timeout
    if (idx == 0)
    {
        return CMD_STAT_TIMEOUT;
    }

    m_rxBufferLen = idx;

    return CMD_STAT_OK;
}

MDBProtocolBase::CmdStatus MDBProtocolBase::parseRecvDataMaster(uint8_t *inData, uint8_t inDataLen,
                                                                uint8_t *outData, uint8_t *outDataLen)
{
    // Response only
    if (inDataLen == 1)
    {
        switch (inData[0])
        {
        case CMD_STAT_RESP_ACK:
            return CMD_STAT_RESP_ACK;
        case CMD_STAT_RESP_NAK:
            return CMD_STAT_RESP_NAK;
        case CMD_STAT_RESP_RET:
            return CMD_STAT_RESP_RET;
        default:
            return CMD_STAT_INVALID_RESP;
        }
    }
    else if (inDataLen > 1)
    {
        uint8_t sum = 0;

        // Check checksum
        for (uint8_t i = 0; i < inDataLen - 1; i++)
        {
            sum += inData[i];
        }
        if (sum != inData[inDataLen - 1])
        {
            return CMD_STAT_INVALID_CHECKSUM;
        }

        // Copy data to output, without checksum
        *outDataLen = inDataLen - 1;

        std::memcpy(outData, inData, *outDataLen);

        return CMD_STAT_OK;
    }

    return CMD_STAT_UNKNOWN_ERROR;
}

MDBProtocolBase::CmdStatus MDBProtocolBase::parseRecvDataSlave(uint8_t *inData, uint8_t inDataLen,
                                                               uint8_t *addr, uint8_t *cmd, 
                                                               int16_t *subCmd, uint8_t *outData, 
                                                               uint8_t *outDataLen)
{
    // Response only, modeBit not set
    if (inDataLen == 1)
    {
        switch (inData[0])
        {
        case CMD_STAT_RESP_ACK:
            return CMD_STAT_RESP_ACK;
        case CMD_STAT_RESP_NAK:
            return CMD_STAT_RESP_NAK;
        case CMD_STAT_RESP_RET:
            return CMD_STAT_RESP_RET;
        default:
            return CMD_STAT_INVALID_RESP;
        }
    }
    else if (inDataLen > 1) // Extract address, cmd, and data (checksum)
    {
        uint8_t sum = 0, idx = 0;

        // Check checksum
        for (uint8_t i = 0; i < inDataLen - 1; i++)
        {
            sum += inData[i];
        }
        if (sum != inData[inDataLen - 1])
        {
            return CMD_STAT_INVALID_CHECKSUM;
        }

        // address and cmd
        *addr = inData[idx] & MDB_FRAME_ADDR_BITMASK;
        *cmd = inData[idx++] & MDB_FRAME_CMD_BITMASK;

        // If we know that we use subCmd mode (indicated by subCmd = -1),
        // we read this byte as subCmd
        if (*subCmd >= 0)
        {
            // Replace current subCmd, need to be re-initialized after being used
            *subCmd = inData[idx++];
        }

        // Copy data without checksum and addr/cmd
        *outDataLen = inDataLen - idx - 1;
        if (*outDataLen > 0)
        {
            std::memcpy(outData, (uint8_t *)&inData[idx], *outDataLen);
        }

        return CMD_STAT_OK;
    }

    return CMD_STAT_UNKNOWN_ERROR;
}