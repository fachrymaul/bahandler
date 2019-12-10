#include <MDBProtocolConv.hpp>
#include <Logger.hpp>

MDBProtocolConv::MDBProtocolConv()
{
}

MDBProtocolConv::~MDBProtocolConv()
{
}

MDBProtocolBase::CmdStatus MDBProtocolConv::transmitPacket(uint8_t *data, uint8_t dataLen,
                                                           ProtocolMode mode, MessageType mType)
{
    if (!m_mutex.try_lock())
    {
        return CMD_STAT_COM_BUSY;
    }

    uint8_t idx = 0;

    // Insert header and cmd allocation, moving data
    std::memmove((uint8_t *)&data[2], data, dataLen);

    // Header
    data[idx++] = MDB_CONV_PROTO_HDR;

    if (mode == MDB_PROTO_MASTER)
    {
        if (mType == MDB_MSG_CMD)
        {
            // CMD and MDB frame len
            // -- CMD: bit 0-2, LEN: bit 3-7
            data[idx++] = ((dataLen << 2) & 0xFC) | (MDB_CONV_CMD_M2S_DATA & 0x03);
        }
        else if (mType == MDB_MSG_RESP)
        {
            // CMD and MDB frame len
            // -- CMD: bit 0-2, LEN: bit 3-7
            data[idx++] = ((dataLen << 2) & 0xFC) | (MDB_CONV_CMD_M2S_RESP & 0x03);
        }
    }
    else if (mode == MDB_PROTO_SLAVE)
    {
        if (mType == MDB_MSG_CMD)
        {
            // CMD and MDB frame len
            // -- CMD: bit 0-2, LEN: bit 3-7
            data[idx++] = ((dataLen << 2) & 0xFC) | (MDB_CONV_CMD_S2M_DATA & 0x03);
        }
        else if (mType == MDB_MSG_RESP)
        {
            // CMD and MDB frame len
            // -- CMD: bit 0-2, LEN: bit 3-7
            data[idx++] = ((dataLen << 2) & 0xFC) | (MDB_CONV_CMD_S2M_RESP & 0x03);
        }
    }

    // MDB Frame, skip as we already move the data
    idx += dataLen;

    // CRC-16
    uint16_t crc = calculateCRC16ccitt(data, idx);
    data[idx++] = crc >> 8;
    data[idx++] = crc;

    // start transmission
    // No need to send 9-bit
    m_pCom->setModeBit(false);
    if (m_pCom->transmit(data, idx) < 1)
        return CMD_STAT_COM_ERROR;

    m_mutex.unlock();

    return CMD_STAT_OK;
}

MDBProtocolBase::CmdStatus MDBProtocolConv::parseRecvDataMaster(uint8_t *inData, uint8_t inDataLen,
                                                                uint8_t *outData, uint8_t *outDataLen)
{
    uint8_t idx = 0, len = 0, cmdFrame = 0xFF;

    // Check Header
    if (inData[idx++] != MDB_CONV_PROTO_HDR)
        return CMD_STAT_UNKNOWN_DATAFRAME;

    // // Check CRC at first place
    // uint16_t crcCalc = calculateCRC16ccitt(inData, inDataLen - 2);
    // if (crcCalc != ((uint16_t)inData[inDataLen - 2] << 8 | inData[inDataLen - 1]))
    //     return CMD_STAT_INVALID_CHECKSUM;

    // Get command and MDB frame length
    len = (inData[idx] >> 2) & 0x3F;
    cmdFrame = inData[idx++] & 0x03;

    // Parse MDB frame using base class
    if (cmdFrame == MDB_CONV_CMD_S2M_RESP || cmdFrame == MDB_CONV_CMD_S2M_DATA)
        return MDBProtocolBase::parseRecvDataMaster((uint8_t *)&inData[idx], len, outData, outDataLen);

    return CMD_STAT_UNKNOWN_ERROR;
}

MDBProtocolBase::CmdStatus MDBProtocolConv::parseRecvDataSlave(uint8_t *inData, uint8_t inDataLen,
                                                               uint8_t *addr, uint8_t *cmd,
                                                               int16_t *subCmd, uint8_t *outData,
                                                               uint8_t *outDataLen)
{
    uint8_t idx = 0, len = 0, cmdFrame = 0xFF;

    // Check Header
    if (inData[idx++] != MDB_CONV_PROTO_HDR)
        return CMD_STAT_UNKNOWN_DATAFRAME;

    // // Check CRC at first place
    // uint16_t crcCalc = calculateCRC16ccitt(inData, inDataLen - 2);
    // if (crcCalc != ((uint16_t)inData[inDataLen - 2] << 8 | inData[inDataLen - 1]))
    //     return CMD_STAT_INVALID_CHECKSUM;

    // Get command and MDB frame length
    len = (inData[idx] >> 2) & 0x3F;
    cmdFrame = inData[idx++] & 0x03;

    // Parse MDB frame using base class
    if (cmdFrame == MDB_CONV_CMD_M2S_RESP || cmdFrame == MDB_CONV_CMD_M2S_DATA)
        return MDBProtocolBase::parseRecvDataSlave((uint8_t *)&inData[idx], len, addr, cmd, subCmd,
                                                   outData, outDataLen);

    return CMD_STAT_UNKNOWN_ERROR;
}

uint16_t MDBProtocolConv::calculateCRC16ccitt(uint8_t *buf, uint16_t len)
{
    register uint16_t counter;
    register uint16_t crc = 0xFFFF;
    for (counter = 0; counter < len; counter++)
        crc = (crc << 8) ^ CRC16_TABLE[((crc >> 8) ^ *buf++) & 0x00FF];
    return crc;
}