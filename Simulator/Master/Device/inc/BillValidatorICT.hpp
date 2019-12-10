#ifndef BILL_VALIDATOR_ICT_H
#define BILL_VALIDATOR_ICT_H

#include <IPeriphInterface.hpp>
#include <ICTProtocol.hpp>

class BillValidatorICT
{
  private:
    enum BillTypeValue {
        ICT_VAL_BILLTYPE_1 = 2000,
        ICT_VAL_BILLTYPW_2 = 5000,
        ICT_VAL_BILLTYPW_3 = 10000,
        ICT_VAL_BILLTYPW_4 = 20000,
        ICT_VAL_BILLTYPW_5 = 50000
    };

  public:
    BillValidatorICT();
    ~BillValidatorICT();

    void initialize(ICTProtocol *pICTProto, ISerial *pCom, ITimer *pTimer);
    void run();
    void update();

  private:
    bool powerUp();
    bool escrow();
    bool poll();
    bool enableDevice(bool enabled);
    bool reset();

    uint64_t getCredit();
    void clearCredit();

  private:
    ISerial *m_pSerial;
    ITimer *m_pTimer;
    ICTProtocol *m_ICTproto;

    uint8_t m_respValue;
    ICTProtocol::RespValue m_resp;

    // Bills
    uint64_t m_credits;
    uint8_t m_billTypeValue;
    uint16_t m_validBills;
    uint16_t m_acceptedBills;
    uint16_t m_rejectedBills;
};

#endif