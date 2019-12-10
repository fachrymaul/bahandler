#ifndef BILL_VALIDATOR_H
#define BILL_VALIDATOR_H

#include <IDeviceMDB.hpp>
#include <IPeriphInterface.hpp>

class BillValidator : public IDeviceMDB
{
  private:
    // Properties
    static const uint8_t MDB_DEV_BILL_ADDRESS = 0x30;
    static const uint8_t MDB_DEV_BILL_MAX_TYPE_CREDIT = 16;

    // Expected Response size
    static const uint8_t MDB_DEV_BILL_RESP_SIZE_ACK = 1;
    static const uint8_t MDB_DEV_BILL_RESP_SIZE_POLL = 16;
    static const uint8_t MDB_DEV_BILL_RESP_SIZE_SETUP = 27;
    static const uint8_t MDB_DEV_BILL_RESP_SIZE_STACKER = 2;

    // Timing
    static const uint16_t MDB_DEV_BILL_DELAY_MS = 500;

    // Command Length
    static const uint8_t MDB_CMD_BILL_TYPE_LEN = 4;
    static const uint8_t MDB_CMD_BILL_ESCROW_LEN = 1;
    static const uint8_t MDB_CMD_BILL_SECURITY_LEN = 2;

    enum CmdMDBBillAcceptor
    {
        MDB_CMD_BILL_SECURITY = 0x02,
        MDB_CMD_BILL_ESCROW = 0x05,
        MDB_CMD_BILL_STACKER = 0x06
    };

    enum RespPollBillAcceptor
    {
        MDB_BILL_RESP_OK = 0x00,
        MDB_BILL_RESP_ERROR = 0x01,
        MDB_BILL_RESP_JUST_RESET = 0x0B
    };

  private:
    // Bill validator default config value
    const uint16_t MDB_DEV_BILL_COUNTRY_CODE = 0x1360;
    const uint16_t MDB_DEV_BILL_SCALING = 1000;
    const uint8_t MDB_DEV_BILL_FEATURE_LEVEL = 1;
    const uint16_t MDB_DEV_BILL_STACKCER_CAPACITY = 400;
    const uint16_t MDB_DEV_BILL_SECURITY_LEVEL = 0b0000000000000010;
    const uint8_t MDB_DEV_BILL_DECIMAL = 0;
    const uint8_t MDB_DEV_BILL_TYPE[MDB_DEV_BILL_MAX_TYPE_CREDIT] = {0x01, 0x02, 0x05, 0x0A,
                                                                     0x14, 0x32, 0x64, 0x00,
                                                                     0x00, 0x00, 0x00, 0x00,
                                                                     0x00, 0x00, 0x00, 0x00};
    const bool MDB_DEV_BILL_ESCROW = true;
    
    // Expansion identification
    const uint8_t *MDB_DEV_BILL_MANUFACTURER_CODE = (uint8_t *)"ICT";
    const uint8_t *MDB_DEV_BILL_SERIAL_NO = (uint8_t *)"244466666888";
    const uint8_t *MDB_DEV_BILL_MODEL_NO = (uint8_t *)"113333555555";
    const uint8_t MDB_DEV_BILL_SW_VERSION[MDB_DEV_SW_LEN] = {0x45, 0x13};

  public:
    BillValidator();
    ~BillValidator();
    void initialize(MDBProtocolBase *mdb, ISerial *pCom, ITimer *pTimer);
    bool reset();
    void print();
    void listen();

    uint64_t getCredit();
    void clearCredit();

  private:
    int16_t poll();
    bool setup(uint8_t numRetries = 0);
    bool setSecurity(uint8_t numRetries = 0);
    bool setBillType(uint8_t numRetries = 0);
    bool getStackerStatus(uint8_t numRetries = 0);
    bool escrow(uint8_t numRetries = 0);

    void resetDevice();

    uint8_t m_devAddress;
    uint8_t m_devSecurity;
    uint8_t m_devEscrow;
    uint8_t m_devStacker;

    uint64_t m_credit;
    uint64_t m_change;

    bool m_full;
    uint16_t m_billsInStacker;
    bool m_billInEscrow;
    bool m_billInEscrowAccept;

    // Bill properties setup
    uint16_t m_billScalingFactor;
    uint8_t m_decimalPlaces;
    uint16_t m_stackerCapacity;
    uint16_t m_securityLevels;
    bool m_canEscrow;
    uint8_t m_billTypeCredit[MDB_DEV_BILL_MAX_TYPE_CREDIT];
    uint16_t m_billTypeEnabled;
    uint16_t m_billEscrowEnabled;

    // Bill reset flags
    bool m_resetFlag;
    bool m_resetFlagAcked;
};

#endif
