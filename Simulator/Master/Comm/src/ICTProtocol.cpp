#include <ICTProtocol.hpp>

ICTProtocol::ICTProtocol()
{
    m_protoState = ICT_STATE_IDLE;
    m_pSerial = nullptr;
    m_pTimer = nullptr;
    m_char = 0x00;
}

ICTProtocol::~ICTProtocol()
{
}

void ICTProtocol::initialize(ISerial *pCom, ITimer *pTimer)
{
    m_pSerial = pCom;
    m_pTimer = pTimer;
}

ICTProtocol::RespValue ICTProtocol::processDataFromDevice(uint8_t *valueOut)
{
    if(m_pSerial->receive(&m_char, 1) <= 0)
        return ICT_RESP_COM_ERROR;
    
    switch (m_protoState)
    {
    case ICT_STATE_IDLE:
        switch (m_char)
        {
            case ICT_CMD_POWER_1:
                m_protoState = ICT_STATE_POWERUP;
                return ICT_RESP_POWERUP;
            case ICT_CMD_BILL_VALID:
                return ICT_RESP_ESCROW_BILL_VALID;
            case ICT_BILL_TYPE_1:
            case ICT_BILL_TYPE_2:
            case ICT_BILL_TYPE_3:
            case ICT_BILL_TYPE_4:
            case ICT_BILL_TYPE_5:
                *valueOut = m_char;
                return ICT_RESP_ESCROW_BILL_VALUE;
            case ICT_CMD_STACKING:
            case ICT_CMD_REJECTING:
                *valueOut = m_char;
                return ICT_RESP_ESCROW_ACTION;
            case ICT_STAT_MOTOR_FAILURE:
            case ICT_STAT_CHECKSUM_ERROR:
            case ICT_STAT_INVALID_COMMAND:
            case ICT_STAT_RESP_ERROR_EX:
            case ICT_STAT_STACKER_OPEN:
            case ICT_STAT_STACKER_PROBLEM:
            case ICT_STAT_BILL_ENABLE:
            case ICT_STAT_BILL_FISH:
            case ICT_STAT_BILL_INHIBIT:
            case ICT_STAT_BILL_JAM:
            case ICT_STAT_BILL_REJECT:
            case ICT_STAT_BILL_REMOVE:
                *valueOut = m_char;
                return ICT_RESP_POLLING_STATUS;
            default:
                break;
        };
        break;
    case ICT_STATE_POWERUP:
        if ((ICTCmd)m_char == ICT_CMD_POWER_2)
        {
            uint8_t resp = ICT_CMD_ACK;
            m_pSerial->setModeBit(false);
            m_protoState = ICT_STATE_IDLE;
            
            if(m_pSerial->transmit(&resp, 1) <= 0)
                return ICT_RESP_COM_ERROR;
        }
        break;
    default:
        break;
    }

    return ICT_RESP_COM_ERROR;
}

bool ICTProtocol::sendCommandToDevice(ICTCmd cmd)
{
    uint8_t resp = cmd;
    m_pSerial->setModeBit(false);
    if(m_pSerial->transmit(&resp, 1) <= 0)
        return false;

    return true;    
}