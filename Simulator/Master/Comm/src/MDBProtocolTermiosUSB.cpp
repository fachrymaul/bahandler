#include <MDBProtocolTermiosUSB.hpp>
#include <cstring>

MDBProtocolTermiosUSB::MDBProtocolTermiosUSB()
{
}

MDBProtocolTermiosUSB::~MDBProtocolTermiosUSB()
{
}

MDBProtocolBase::CmdStatus MDBProtocolTermiosUSB::parseRecvDataMaster(uint8_t *inData, uint8_t inDataLen,
                                                                        uint8_t *outData, uint8_t *outDataLen)
{
    uint8_t modebitHdrLen = sizeof((uint16_t)MDB_TERMIOS_MODEBIT_HDR) / sizeof(uint8_t);

    // Response only
    if (inDataLen == modebitHdrLen + 1)
    {
        if ((((uint16_t)inData[0] << 8) | inData[1]) == (uint16_t)MDB_TERMIOS_MODEBIT_HDR)
        {
            switch (inData[modebitHdrLen])
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
    }
    else if (inDataLen > modebitHdrLen + 1)
    {
        const uint8_t oDataLen = inDataLen;
        uint8_t oData[oDataLen];
        uint8_t sum = 0, idx = 0;

        std::memset(oData, 0x00, oDataLen);

        // Strip off HDR bytes
        for(uint8_t i = 0; i < inDataLen; i++)
        {
            if ((((uint16_t)inData[i] << 8) | inData[i + 1]) == (uint16_t)MDB_TERMIOS_MODEBIT_HDR)
            {
                oData[idx++] = inData[i + modebitHdrLen];
                i += modebitHdrLen;
            }
            else
            {
                oData[idx++] = inData[i];
            }
        }

        // Check checksum
        for (int i = 0; i < idx - 1; i++)
        {
            sum += oData[i];
        }

        if (sum != oData[idx - 1])
        {
            return CMD_STAT_INVALID_CHECKSUM;
        }

        // Copy data to output, without checksum
        *outDataLen = idx - 1;
        if (*outDataLen > 0)
        {
            std::memcpy(outData, oData, *outDataLen);
        }

        return CMD_STAT_OK;
    }

    return CMD_STAT_UNKNOWN_ERROR;
}

MDBProtocolBase::CmdStatus MDBProtocolTermiosUSB::parseRecvDataSlave(uint8_t *inData, uint8_t inDataLen,
                                                                       uint8_t *addr, uint8_t *cmd, 
                                                                       int16_t *subCmd, uint8_t *outData, 
                                                                       uint8_t *outDataLen)
{
    uint8_t modebitHdrLen = sizeof((uint16_t)MDB_TERMIOS_MODEBIT_HDR) / sizeof(uint8_t);

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
    else if (inDataLen > modebitHdrLen + 1) // Extract address, cmd, and data (checksum)
    {
        // address and cmd
        if ((((uint16_t)inData[0] << 8) | inData[1]) == (uint16_t)MDB_TERMIOS_MODEBIT_HDR)
        {
            const uint8_t oDataLen = inDataLen - modebitHdrLen;
            uint8_t oData[oDataLen];
            uint8_t idx = 0, sum = 0;

            std::memset(oData, 0x00, oDataLen);
            std::memcpy(oData, (uint8_t *)&inData[modebitHdrLen], oDataLen);

            // Check checksum
            for (int i = 0; i < oDataLen - 1; i++)
            {
                sum += oData[i];
            }
            if (sum != oData[oDataLen - 1])
            {
                return CMD_STAT_INVALID_CHECKSUM;
            }

            *addr = oData[idx] & MDB_FRAME_ADDR_BITMASK;
            *cmd = oData[idx++] & MDB_FRAME_CMD_BITMASK;

            // If we know that we use subCmd mode (indicated by subCmd = -1),
            // we read this byte as subCmd
            if (*subCmd >= 0)
            {
                // Replace current subCmd, need to be re-initialized after being used
                *subCmd = oData[idx++];
            }

            // Copy data without checksum
            *outDataLen = oDataLen - idx - 1;
            if (*outDataLen > 0)
            {
                std::memcpy(outData, (uint8_t *)&oData[idx], *outDataLen);
            }

            return CMD_STAT_OK;
        }
        else
        {
            return CMD_STAT_UNKNOWN_DATAFRAME;
        }
    }

    return CMD_STAT_UNKNOWN_ERROR;
}
