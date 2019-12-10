#ifndef BILL_VALIDATOR_MDB_H
#define BILL_VALIDATOR_MDB_H

#include <IDeviceMDB.hpp>
#include <IPeriphInterface.hpp>

class BillValidatorMDB : public IDeviceMDB
{
  // private:
  public:
    // Properties
    static const uint8_t MDB_DEV_BILL_ADDRESS = 0x30;
    static const uint8_t MDB_DEV_BILL_MAX_TYPE_CREDIT = 16;

    // Expected Response size
    static const uint8_t MDB_DEV_BILL_RESP_SIZE_ACK = 1;
    static const uint8_t MDB_DEV_BILL_RESP_SIZE_POLL = 16;
    static const uint8_t MDB_DEV_BILL_RESP_SIZE_SETUP = 27;
    static const uint8_t MDB_DEV_BILL_RESP_SIZE_STACKER = 2;

    // Timing
    static const uint16_t MDB_DEV_BILL_DELAY_MS = 1000;

    // Command Length
    static const uint8_t MDB_CMD_BILL_TYPE_LEN = 4;
    static const uint8_t MDB_CMD_BILL_ESCROW_LEN = 1;
    static const uint8_t MDB_CMD_BILL_SECURITY_LEN = 2;

    // Sub-command Length
    static const uint8_t MDB_SUBCMD_LVL1_ID_LEN = 29;
    static const uint8_t MDB_SUBCMD_LVL2_FEATURE_EN_LEN = 4;
    static const uint8_t MDB_SUBCMD_LVL2_ID_LEN = 33;
    static const uint8_t MDB_SUBCMD_RECYCLER_SETUP_LEN = 2;
    static const uint8_t MDB_SUBCMD_RECYCLER_ENABLE_LEN = 19;
    static const uint8_t MDB_SUBCMD_BILL_DISPENSE_STATUS_LEN = 34;
    static const uint8_t MDB_SUBCMD_DISPENSE_BILL_LEN = 3;
    static const uint8_t MDB_SUBCMD_DISPENSE_VALUE_LEN = 2;
    static const uint8_t MDB_SUBCMD_PAYOUT_STATUS_LEN = 32;
    static const uint8_t MDB_SUBCMD_PAYOUT_VALUE_POLL_LEN = 2;

    // Sub-command response length
    static const uint8_t MDB_DEV_LVL2_OPT_LEN = 4;

    enum CmdMDBBillAcceptor
    {
        MDB_CMD_BILL_SECURITY = 0x02,
        MDB_CMD_BILL_ESCROW = 0x05,
        MDB_CMD_BILL_STACKER = 0x06
    };

    enum SubCmdMDBBillAcceptor
    {
        MDB_SUBCMD_LVL1_ID = 0x00,
        MDB_SUBCMD_LVL2_FEATURE_EN = 0x01,
        MDB_SUBCMD_LVL2_ID = 0x02,
        MDB_SUBCMD_RECYCLER_SETUP = 0x03,
        MDB_SUBCMD_RECYCLER_ENABLE = 0x04,
        MDB_SUBCMD_BILL_DISPENSE_STATUS = 0x05,
        MDB_SUBCMD_DISPENSE_BILL = 0x06,
        MDB_SUBCMD_DISPENSE_VALUE = 0x07,
        MDB_SUBCMD_PAYOUT_STATUS = 0x08,
        MDB_SUBCMD_PAYOUT_VALUE_POLL = 0x09,
        MDB_SUBCMD_PAYOUT_CANCEL = 0x0A,
        MDB_SUBCMD_FTL_REQ_RECV = 0xFA,
        MDB_SUBCMD_FTL_RETRY_DENY = 0xFB,
        MDB_SUBCMD_FTL_SEND_BLOCK = 0xFC,
        MDB_SUBCMD_FTL_OK_SEND = 0xFD,
        MDB_SUBCMD_FTL_REQ_SEND = 0xFE,
        MDB_SUBCMD_FTL_DIAG = 0xFF
    };

    enum RespPollBillAcceptor
    {
        MDB_BILL_RESP_OK = 0x00,
        MDB_BILL_RESP_ERROR = 0x01,
        MDB_BILL_RESP_JUST_RESET = 0x0B
    };

  public:
    BillValidatorMDB();
    ~BillValidatorMDB();
    void initialize(MDBProtocolBase *mdb, ISerial *pCom, ITimer *pTimer);
    bool reset();
    void print();
    bool update();

    uint64_t getCredit();
    void clearCredit();
  // private:
  public:
    int16_t poll();
    bool setup(uint8_t numRetries = 0);
    bool run();
    bool setSecurity(uint8_t numRetries = 0);
    bool setBillType(uint8_t *billType, uint8_t numRetries = 0);
    bool getStackerStatus(uint8_t numRetries = 0);
    bool escrow(bool acceptBill, uint8_t numRetries = 0);

  private:
    bool scGetLvl1Id(uint8_t numRetries = 0);
    bool scSetLvl2FeatureEnable(uint8_t *featureBytes,
                                uint8_t len = MDB_DEV_LVL2_OPT_LEN,
                                uint8_t numRetries = 0);
    bool scGetLvl2Id(uint8_t numRetries = 0);
    bool scGetDispenseValue(uint8_t numRetries = 0);

  private:
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

    // Bill properties level 2
    bool m_ftlSupport;
    bool m_billRecyclerSupport;

    // Bill reset flags
    bool m_resetFlag;
    bool m_resetFlagAcked;
};

#endif
