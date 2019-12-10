#include <BillValidatorICT.hpp>
#include <Logger.hpp>

BillValidatorICT::BillValidatorICT()
{
    m_pSerial = nullptr;
    m_pTimer = nullptr;
    m_ICTproto = nullptr;
    m_respValue = 0x00;
    m_resp = ICTProtocol::ICT_RESP_COM_ERROR;
}

BillValidatorICT::~BillValidatorICT()
{
}

void BillValidatorICT::initialize(ICTProtocol *pICTProto, ISerial *pCom, ITimer *pTimer)
{
    m_pSerial = pCom;
    m_pTimer = pTimer;
    m_ICTproto = pICTProto;
    m_ICTproto->initialize(pCom, pTimer);
}

void BillValidatorICT::run()
{
}

void BillValidatorICT::update()
{
    // Listen for data
    if (m_pSerial->hasData())
    {
        m_resp = m_ICTproto->processDataFromDevice(&m_respValue);

        switch (m_resp)
        {
            case ICTProtocol::ICT_RESP_ESCROW_BILL_VALID:
                m_validBills++;
                break;
            case ICTProtocol::ICT_RESP_ESCROW_BILL_VALUE:
                m_billTypeValue = m_respValue;
                debug << "BV: Bill Type Value = " << m_billTypeValue << "\n";
                break;
            case 
            case ICTProtocol::ICT_RESP_ESCROW_ACTION:
                break;
            default:
                break;
        }
    }
}
