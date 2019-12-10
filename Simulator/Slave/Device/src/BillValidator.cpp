#include <BillValidator.hpp>
#include <cstring>

BillValidator::BillValidator()
{
    m_devAddress = MDB_DEV_BILL_ADDRESS;
    m_devSecurity = MDB_CMD_BILL_SECURITY;
    m_devEscrow = MDB_CMD_BILL_ESCROW;
    m_devStacker = MDB_CMD_BILL_STACKER;

    m_manufacturerCode = (uint8_t *)MDB_DEV_BILL_MANUFACTURER_CODE;
    m_modelNumber = (uint8_t *)MDB_DEV_BILL_MODEL_NO;
    m_serialNumber = (uint8_t *)MDB_DEV_BILL_SERIAL_NO;
    m_softwareVersion = (uint8_t *)MDB_DEV_BILL_SW_VERSION;

    m_pMDB = nullptr;
    m_pSerial = nullptr;
    m_pTimer = nullptr;
    m_outBufferLen = 0;
    std::memset(m_outBuffer, 0x00, sizeof(m_outBuffer));

    m_full = false;
    m_billsInStacker = 0;
    m_resetCount = 0;

    m_resetFlag = false;
    m_resetFlagAcked = false;

    resetDevice();
}

BillValidator::~BillValidator()
{
}

void BillValidator::initialize(MDBProtocolBase *mdb, ISerial *pCom, ITimer *pTimer)
{
    m_pMDB = mdb;
    m_pSerial = pCom;
    m_pTimer = pTimer;
    m_pMDB->initialize(m_pSerial, m_pTimer);
}

void BillValidator::resetDevice()
{
    m_credit = 0;
    m_change = 0;

    m_resetCount++;

    m_billScalingFactor = MDB_DEV_BILL_SCALING;
    m_decimalPlaces = MDB_DEV_BILL_DECIMAL;
    m_stackerCapacity = MDB_DEV_BILL_STACKCER_CAPACITY;
    m_securityLevels = MDB_DEV_BILL_SECURITY_LEVEL;
    m_canEscrow = MDB_DEV_BILL_ESCROW;
    std::memcpy(m_billTypeCredit, MDB_DEV_BILL_TYPE, MDB_DEV_BILL_MAX_TYPE_CREDIT);

    m_billTypeEnabled = 0b0000000000111111;
    m_billEscrowEnabled = 0b0000000000111111;
    m_billInEscrowAccept = false;
}

void BillValidator::listen()
{
    if (m_pSerial->hasData())
    {
        uint8_t addr = 0;
        uint8_t cmd = 0;
        int16_t subCmd = -1;

        MDBProtocolBase::CmdStatus stat = m_pMDB->getDataFromMaster(m_outBuffer, &m_outBufferLen, &addr,
                                                                       &cmd, &subCmd);

        // Command received from master
        if (stat == MDBProtocolBase::CMD_STAT_OK)
        {
            // Check address
            if (addr != MDB_DEV_BILL_ADDRESS)
            {
                warning << "BV: Invalid address"
                        << "\n";
                return;
            }

            // Parse and match command
            switch (cmd)
            {
            case MDB_CMD_RESET:
                reset();
                break;
            case MDB_CMD_SETUP:
                setup();
                break;
            case MDB_CMD_BILL_SECURITY:
                setSecurity();
                break;
            case MDB_CMD_POLL:
                poll();
                break;
            case MDB_CMD_TYPE:
                setBillType();
                break;
            case MDB_CMD_BILL_ESCROW:
                escrow();
                break;
            case MDB_CMD_BILL_STACKER:
                getStackerStatus();
                break;
            default:
                warning << "BV: Invalid command "
                        << "\n";
                break;
            }
        }
        // Response received from master
        else if (stat == MDBProtocolBase::CMD_STAT_RESP_ACK ||
                 stat == MDBProtocolBase::CMD_STAT_RESP_NAK ||
                 stat == MDBProtocolBase::CMD_STAT_RESP_RET)
        {
            if (stat == MDBProtocolBase::CMD_STAT_RESP_ACK)
            {
                // Get ACK from master after reset
                if (m_resetFlag == true && m_resetFlagAcked == false)
                {
                    debug << "BV: Get ACK from master after reset"
                          << "\n";
                    m_resetFlagAcked = true;
                    m_resetFlag = false;
                }
            }
        }
    }
}

void BillValidator::print()
{
}

uint64_t BillValidator::getCredit()
{
    return m_credit;
}

void BillValidator::clearCredit()
{
    m_credit = 0;
}

bool BillValidator::reset()
{
    debug << "BV: Init reset"
          << "\n";

    m_resetFlag = true;
    m_resetFlagAcked = false;
    resetDevice();

    if (m_pMDB->sendResponseFromSlave(MDBProtocolBase::CMD_STAT_RESP_ACK) != MDBProtocolBase::CMD_STAT_OK)
        return false;

    debug << "BV: ACK Reset resp transmitted"
          << "\n";

    return true;
}

bool BillValidator::setup(uint8_t numRetries)
{
    debug << "BV: Init setup"
          << "\n";

    uint8_t checksum = 0, idx = 0;

    // Z1: Feature Level
    checksum += m_outBuffer[idx++] = m_featureLevel;

    // Z2-Z3: Country/Currency code
    checksum += m_outBuffer[idx++] = (m_country >> 8) & 0xFF;
    checksum += m_outBuffer[idx++] = m_country & 0xFF;

    // Z4-Z5: Bill scaling factor
    checksum += m_outBuffer[idx++] = (m_billScalingFactor >> 8) & 0xFF;
    checksum += m_outBuffer[idx++] = m_billScalingFactor & 0xFF;

    // Z6: Decimal places
    checksum += m_outBuffer[idx++] = m_decimalPlaces;

    // Z7-Z8: Stacker capacity
    checksum += m_outBuffer[idx++] = (m_stackerCapacity >> 8) & 0xFF;
    checksum += m_outBuffer[idx++] = m_stackerCapacity & 0xFF;

    // Z9-Z10: Bill security levels
    checksum += m_outBuffer[idx++] = (m_securityLevels >> 8) & 0xFF;
    checksum += m_outBuffer[idx++] = m_securityLevels & 0xFF;

    // Z11: Escrow/No Escrow
    checksum += m_outBuffer[idx++] = m_canEscrow;

    // Z12-Z27: Bill Type Credit
    for (uint8_t i = 0; i < MDB_DEV_BILL_MAX_TYPE_CREDIT; i++)
    {
        checksum += m_outBuffer[idx++] = MDB_DEV_BILL_TYPE[i];
    }

    // Checksum
    m_outBuffer[idx++] = checksum;

    if (m_pMDB->sendDataFromSlave(m_outBuffer, idx) != MDBProtocolBase::CMD_STAT_OK)
    {
        error << "BV: Could not send Setup data to master"
              << "\n";
        return false;
    }

    debug << "BV: Setup completed"
          << "\n";
    return true;
}

bool BillValidator::setSecurity(uint8_t numRetries)
{
    debug << "BV: Set Security"
          << "\n";

    if (m_outBufferLen != MDB_CMD_BILL_SECURITY_LEN)
    {
        error << "BV: Invalid Security data len"
              << "\n";
        return false;
    }

    m_securityLevels = ((uint16_t)(m_outBuffer[0]) << 8 | m_outBuffer[1]);

    if (m_pMDB->sendResponseFromSlave(MDBProtocolBase::CMD_STAT_RESP_ACK) != MDBProtocolBase::CMD_STAT_OK)
    {
        error << "BV: Could not send ACK Security resp to master"
              << "\n";
        return false;
    }

    debug << "BV: ACK Security resp transmitted"
          << "\n";

    return true;
}

int16_t BillValidator::poll()
{
    debug << "BV: Poll status"
          << "\n";

    uint8_t randNumber = std::rand() % 4;
    uint8_t respData = 0x00;

    if (m_resetFlag == true && m_resetFlagAcked == false)
    {
        // Just reset
        respData = 0b00000110;
        if (m_pMDB->sendDataFromSlave(&respData, sizeof(respData)) != MDBProtocolBase::CMD_STAT_OK)
        {
            error << "BV: Could not send Poll data to master"
                  << "\n";
            return false;
        }

        debug << "BV: Poll Just Reset resp sent"
              << "\n";
        return true;
    }
    else if (m_resetFlagAcked == true)
    {
        if (randNumber == 0)
        {
            // Bill type 0, stacked
            respData = 0b10000000;
            debug << "BV: Bill type 0 accepted"
                  << "\n";
            m_billsInStacker++;
        }
        else if (randNumber == 1)
        {
            // Bill type 1, reject
            respData = 0b10010001;
            debug << "BV: Bill type 1 rejected"
                  << "\n";
        }
        else if (randNumber == 2)
        {
            // Bill type 2, stacked
            respData = 0b10000010;
            m_billsInStacker++;
            debug << "BV: Bill type 2 accepted"
                  << "\n";
        }
        else
        {
            if (m_pMDB->sendResponseFromSlave(MDBProtocolBase::CMD_STAT_RESP_ACK) != MDBProtocolBase::CMD_STAT_OK)
            {
                error << "BV: Could not send Poll resp to master"
                      << "\n";
                return false;
            }

            debug << "BV: Poll resp sent"
                  << "\n";
            return true;
        }

        if (m_pMDB->sendDataFromSlave(&respData, sizeof(respData)) != MDBProtocolBase::CMD_STAT_OK)
        {
            error << "BV: Could not send Poll data to master"
                  << "\n";
            return false;
        }
    }
    else
    {
        if (m_pMDB->sendResponseFromSlave(MDBProtocolBase::CMD_STAT_RESP_ACK) != MDBProtocolBase::CMD_STAT_OK)
        {
            error << "BV: Could not send Poll resp to master"
                  << "\n";
            return false;
        }
    }

    debug << "BV: Poll resp sent"
          << "\n";
    return true;
}

bool BillValidator::setBillType(uint8_t numRetries)
{
    debug << "BV: Set Bill Type"
          << "\n";

    if (m_outBufferLen != MDB_CMD_BILL_TYPE_LEN)
    {
        error << "BV: Invalid Bill Type data len"
              << "\n";
        return false;
    }

    m_billTypeEnabled = ((uint16_t)(m_outBuffer[0]) >> 8 | m_outBuffer[1]);
    m_billInEscrow = ((uint16_t)(m_outBuffer[2]) >> 8 | m_outBuffer[3]);

    if (m_pMDB->sendResponseFromSlave(MDBProtocolBase::CMD_STAT_RESP_ACK) != MDBProtocolBase::CMD_STAT_OK)
    {
        error << "BV: Could not send ACK Setup resp to master"
              << "\n";
        return false;
    }

    debug << "BV: ACK Bill Type resp transmitted"
          << "\n";

    return true;
}

bool BillValidator::escrow(uint8_t numRetries)
{
    debug << "BV: Set Escrow"
          << "\n";

    if (m_outBufferLen != MDB_CMD_BILL_ESCROW_LEN)
    {
        error << "BV: Invalid Escrow data len"
              << "\n";
        return false;
    }

    if (m_outBuffer[0] == 0)
        m_billInEscrowAccept = false;
    else
        m_billInEscrowAccept = true;

    if (m_pMDB->sendResponseFromSlave(MDBProtocolBase::CMD_STAT_RESP_ACK) != MDBProtocolBase::CMD_STAT_OK)
    {
        error << "BV: Could not send ACK Escrow resp to master"
              << "\n";
        return false;
    }

    debug << "BV: ACK Escrow resp transmitted"
          << "\n";

    return true;
}

bool BillValidator::getStackerStatus(uint8_t numRetries)
{
    debug << "BV: Get Stacker status"
          << "\n";

    uint8_t respData[] = {0x00, 0x00};
    if (m_full)
    {
        respData[0] |= 0b10000000;
    }
    respData[0] |= m_billsInStacker >> 8;
    respData[1] = m_billsInStacker;

    if (m_pMDB->sendDataFromSlave(respData, sizeof(respData)) != MDBProtocolBase::CMD_STAT_OK)
    {
        error << "BV: Could not send Stacker data to master"
              << "\n";
        return false;
    }

    debug << "BV: Get Stacker status completed"
          << "\n";
    return true;
}
