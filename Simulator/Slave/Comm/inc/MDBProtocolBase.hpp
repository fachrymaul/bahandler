#ifndef MDB_PROTOCOL_BASE_H
#define MDB_PROTOCOL_BASE_H

#include <IPeriphInterface.hpp>
#include <mutex>

class MDBProtocolBase
{
  protected:
    // Timing
    static const uint32_t MDB_TIMEOUT_MS = 1000;
    static const uint32_t MDB_TIMEBREAK_MS = 100;
    static const uint32_t MDB_TIMESETUP_MS = 200;
    static const uint32_t MDB_MAX_IBYTE_TIME = 1;

    // Buffer
    static const uint8_t MDB_MAX_TX_BUFFER_LEN = 72;
    static const uint8_t MDB_MAX_RX_BUFFER_LEN = 72;
    static const uint8_t MDB_SAFETY_BUFFER_LEN = 10; // Handle dummy mode bit like in termios and checksum
                                                     // (must be large than 3)

    // MDB Data frame
    const uint8_t MDB_FRAME_ADDR_BITMASK = 0xF8;
    const uint8_t MDB_FRAME_CMD_BITMASK = 0xFF ^ MDB_FRAME_ADDR_BITMASK;

    enum ProtocolMode
    {
        MDB_PROTO_MASTER = 0x00,
        MDB_PROTO_SLAVE = 0x01
    };

    enum MessageType
    {
        MDB_MSG_CMD = 0x00,
        MDB_MSG_RESP = 0x01
    };

  public:
    enum CmdStatus
    {
        // MDB response status
        CMD_STAT_RESP_ACK = 0x00,
        CMD_STAT_RESP_RET = 0xAA,
        CMD_STAT_RESP_NAK = 0xFF,

        // Com status
        CMD_STAT_OK = 0x01,
        CMD_STAT_COM_ERROR = 0xB0,
        CMD_STAT_COM_BUSY = 0xB1,
        CMD_STAT_BUF_OVERFLOW = 0xC0,
        CMD_STAT_TIMEOUT = 0xD0,

        // Parser
        CMD_STAT_INVALID_RESP = 0x80,
        CMD_STAT_INVALID_CHECKSUM = 0x81,
        CMD_STAT_UNKNOWN_DATAFRAME = 0x82,
        CMD_STAT_UNKNOWN_ERROR = 0xF0
    };

  public:
    MDBProtocolBase();
    ~MDBProtocolBase();

    void initialize(ISerial *pCom, ITimer *pTimer);

    // Master
    CmdStatus sendCommandFromMaster(uint8_t address, uint8_t cmd,
                                    uint8_t *data = nullptr, uint8_t dataLen = 0);
    CmdStatus sendCommandFromMaster(uint8_t address, uint8_t cmd,
                                    int16_t subCmd, uint8_t *data = nullptr, uint8_t dataLen = 0);
    CmdStatus sendResponseFromMaster(CmdStatus resp);
    CmdStatus getResponseFromSlave(uint8_t *data, uint8_t *dataLen,
                                   uint8_t expectedLen = MDB_MAX_RX_BUFFER_LEN,
                                   uint32_t timeoutMs = MDB_TIMEOUT_MS);

    // Slave
    CmdStatus sendDataFromSlave(uint8_t *data, uint8_t dataLen = 0);
    CmdStatus sendResponseFromSlave(CmdStatus resp);
    CmdStatus getResponseFromMaster(uint8_t *data, uint8_t *dataLen,
                                    uint8_t *addr, uint8_t *cmd, int16_t *subCmd,
                                    uint8_t expectedLen = MDB_MAX_RX_BUFFER_LEN,
                                    uint32_t timeoutMs = MDB_TIMEOUT_MS);
    CmdStatus getDataFromMaster(uint8_t *data, uint8_t *dataLen,
                                uint8_t *addr, uint8_t *cmd, int16_t *subCmd,
                                uint8_t expectedLen = MDB_MAX_RX_BUFFER_LEN,
                                uint32_t timeoutMs = MDB_TIMEOUT_MS);

  protected:
    virtual CmdStatus transmitPacket(uint8_t *data, uint8_t dataLen,
                                     ProtocolMode mode, MessageType mType);
    virtual CmdStatus parseRecvDataMaster(uint8_t *inData, uint8_t inDataLen,
                                          uint8_t *outData, uint8_t *outDataLen);
    virtual CmdStatus parseRecvDataSlave(uint8_t *inData, uint8_t inDataLen,
                                         uint8_t *addr, uint8_t *cmd, int16_t *subCmd,
                                         uint8_t *outData, uint8_t *outDataLen);
  private:
    CmdStatus collectData(uint8_t expectedLen = MDB_MAX_RX_BUFFER_LEN,
                        uint32_t timeoutMs = MDB_TIMEOUT_MS);

  protected:
    ISerial *m_pCom;
    ITimer *m_pTimer;

    uint8_t m_txBuffer[MDB_MAX_TX_BUFFER_LEN];
    uint8_t m_rxBuffer[MDB_MAX_RX_BUFFER_LEN];
    uint8_t m_rxBufferLen;

    std::mutex m_mutex;
};

#endif
