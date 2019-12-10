#ifndef ICT_PROTOCOL_H
#define ICT_PROTOCOL_H

#include <IPeriphInterface.hpp>
#include <cstdint>

class ICTProtocol
{
private:
    static const uint32_t ICT_TIMEOUT_POWERUP_MS = 2000;
    static const uint32_t ICT_TIMEOUT_ESCROW_MS = 5000;

    // Command
    enum ICTCmd {
        ICT_CMD_POWER_1 = 0x80,
        ICT_CMD_POWER_2 = 0x8F,
        ICT_CMD_ACK = 0x02,
        ICT_CMD_BILL_VALID = 0x81,
        ICT_CMD_REJECT = 0x0F,
        ICT_CMD_HOLD = 0x18,
        ICT_CMD_STACKING = 0x10,
        ICT_CMD_REJECTING = 0x11,
        ICT_CMD_BILL_STATUS = 0x0C,
        ICT_CMD_ENABLE_BILL = 0x3E,
        ICT_CMD_DISABLE_BILL = 0x5E,
        ICT_CMD_RESET = 0x30
    };

public:
    // Bill Status
    enum ICTBillStatus {
        ICT_STAT_MOTOR_FAILURE = 0x20,
        ICT_STAT_CHECKSUM_ERROR,
        ICT_STAT_BILL_JAM,
        ICT_STAT_BILL_REMOVE,
        ICT_STAT_STACKER_OPEN,
        ICT_STAT_STACKER_PROBLEM,
        ICT_STAT_BILL_FISH,
        ICT_STAT_BILL_REJECT,
        ICT_STAT_INVALID_COMMAND,
        ICT_STAT_RESP_ERROR_EX = 0x2F,
        ICT_STAT_BILL_ENABLE = 0x3E,
        ICT_STAT_BILL_INHIBIT = 0x5E
    };

    // Bill Type
    enum BillType {
        ICT_BILL_TYPE_1 = 0x40,
        ICT_BILL_TYPE_2 = 0x41,
        ICT_BILL_TYPE_3 = 0x42,
        ICT_BILL_TYPE_4 = 0x43,
        ICT_BILL_TYPE_5 = 0x44
    };

    // Protocol State
    enum ProtoState {
        ICT_STATE_IDLE,
        ICT_STATE_POWERUP
    };

    // Resp Value From Master
    enum RespValue {
        ICT_RESP_POWERUP,
        ICT_RESP_ESCROW_BILL_VALID,
        ICT_RESP_ESCROW_BILL_VALUE,
        ICT_RESP_ESCROW_ACTION,
        ICT_RESP_POLLING_STATUS,
        ICT_RESP_COM_ERROR
    };

public:
    ICTProtocol();
    ~ICTProtocol();

    void initialize(ISerial *pCom, ITimer *pTimer);

    RespValue processDataFromDevice(uint8_t *valueOut);
    bool sendCommandToDevice(ICTCmd command);

private:
    uint8_t m_protoState;
    ISerial *m_pSerial;
    ITimer *m_pTimer;
    uint8_t m_char;
};

#endif