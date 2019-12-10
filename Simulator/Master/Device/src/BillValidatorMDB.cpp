#include <BillValidatorMDB.hpp>
#include <BitHelper.hpp>
#include <cstring>

BillValidatorMDB::BillValidatorMDB()
{
    m_devAddress = MDB_DEV_BILL_ADDRESS;
    m_devSecurity = MDB_CMD_BILL_SECURITY;
    m_devEscrow = MDB_CMD_BILL_ESCROW;
    m_devStacker = MDB_CMD_BILL_STACKER;

    m_pMDB = nullptr;
    m_pSerial = nullptr;
    m_pTimer = nullptr;
    m_outBufferLen = 0;
    std::memset(m_outBuffer, 0x00, sizeof(m_outBuffer));

    m_credit = 0;
    m_change = 0;

    m_resetCount = 0;

    m_full = false;
    m_billInEscrow = false;
    m_billScalingFactor = 0;
    m_decimalPlaces = 0;
    m_stackerCapacity = 0;
    m_securityLevels = 0;
    m_canEscrow = false;
    std::memset(m_billTypeCredit, 0x00, sizeof(m_billTypeCredit));
}

BillValidatorMDB::~BillValidatorMDB()
{
}

void BillValidatorMDB::initialize(MDBProtocolBase *mdb, ISerial *pCom, ITimer *pTimer)
{
    m_pMDB = mdb;
    m_pSerial = pCom;
    m_pTimer = pTimer;
    m_pMDB->initialize(m_pSerial, m_pTimer);
}

bool BillValidatorMDB::reset()
{
    uint8_t count = 0;

    debug << "BV: Init reset"
          << "\n";

    // Wait for Bill Validator to response
    while (poll() == MDB_BILL_RESP_ERROR)
    {
        if (count > MDB_DEV_MAX_RESET_POLL)
        {
            debug << "BV: Not connected"
                  << "\n";
            return false;
        }
        m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
        count++;
    }

    count = 0;

    // Wait for BV to power up
    while (poll() != MDB_BILL_RESP_ERROR && count < MDB_DEV_MAX_RESET_POLL)
    {
        debug << "BV_COM: Send RESET cmd"
              << "\n";
        MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS,
                                                                        MDB_CMD_RESET);
        if (stat != MDBProtocolBase::CMD_STAT_OK)
            return false;

        stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_DEV_BILL_RESP_SIZE_ACK);
        if (stat == MDBProtocolBase::CMD_STAT_RESP_ACK)
        {
            // Just reset
            int counterJustReset = 0;

            debug << "BV_COM: Reset cmd ok"
                  << "\n";

            while (poll() != MDB_BILL_RESP_JUST_RESET)
            {
                if (counterJustReset > MDB_DEV_MAX_RESET_POLL)
                {
                    debug << "BV_COM: No JUST_RESET received"
                          << "\n";
                    return false;
                }
                m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
                counterJustReset++;
            }
            debug << "BV: Reset completed"
                  << "\n";
            return true;
        }
        count++;
        m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
    }
    debug << "BV: Reset failed"
          << "\n";
    return false;
}

void BillValidatorMDB::print()
{
}

bool BillValidatorMDB::update()
{
    if (poll() == MDB_BILL_RESP_ERROR)
        return false;
    if (!getStackerStatus())
        return false;

    uint8_t billType[MDB_CMD_BILL_TYPE_LEN] = {0x00, 0b11111111, 0x00, 0x00};

    if (m_billInEscrow)
    {
        if (!escrow(true))
            return false;
    }
    else
    {
        if (!setBillType(billType))
            return false;
    }

    return true;
}

uint64_t BillValidatorMDB::getCredit()
{
    return m_credit;
}

void BillValidatorMDB::clearCredit()
{
    m_credit = 0;
}

int16_t BillValidatorMDB::poll()
{
    bool reset = false;

    debug << "BV_COM: Send POLL cmd\n";
    MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS,
                                                                    MDB_CMD_POLL);

    if (stat != MDBProtocolBase::CMD_STAT_OK)
        return MDB_BILL_RESP_ERROR;

    stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_DEV_BILL_RESP_SIZE_POLL);

    // Get ACK from slave
    if (stat == MDBProtocolBase::CMD_STAT_RESP_ACK)
    {
        debug << "BV_COM: BV poll active"
              << "\n";
        return MDB_BILL_RESP_OK;
    }

    // We receive data from slave, then send ACK
    if (stat == MDBProtocolBase::CMD_STAT_OK && m_outBufferLen > 0)
    {
        debug << "BV_COM: Received STAT from slave"
              << "\n";
        stat = m_pMDB->sendResponseFromMaster(MDBProtocolBase::CMD_STAT_RESP_ACK);
        if (stat != MDBProtocolBase::CMD_STAT_OK)
        {
            error << "BV_COM: Could not send data to slave"
                  << "\n";
            return MDB_BILL_RESP_ERROR;
        }
    }
    else
    {
        error << "BV_COM: Could not receive data from slave"
              << "\n";
        return MDB_BILL_RESP_ERROR;
    }

    // Parse received data
    for (int i = 0; i < m_outBufferLen && i < MDB_DEV_BILL_RESP_SIZE_POLL; i++)
    {
        // Bill Status
        if (m_outBuffer[i] & 0b10000000)
        {
            uint8_t routing = (m_outBuffer[i] & 0b01110000) >> 4;
            uint8_t billType = m_outBuffer[i] & 0b00001111;

            warning << "BV: Bill Type = " << billType << "\n";

            switch (routing)
            {
            case 0:
                debug << "BV: bill credited"
                      << "\n";
                m_credit += (m_billTypeCredit[billType] * m_billScalingFactor);
                // m_credit += billType;
                break;
            case 1:
                debug << "BV: escrow position"
                      << "\n";
                m_billInEscrow = true;
                break;
            case 2:
                debug << "BV: bill returned"
                      << "\n";
                break;
            case 3:
                debug << "BV: bill to recycler"
                      << "\n";
                break;
            case 4:
                debug << "BV: disabled bill rejected"
                      << "\n";
                break;
            case 5:
                debug << "BV: bill to recycler - manual"
                      << "\n";
                break;
            case 6:
                debug << "BV: manual dispense"
                      << "\n";
                break;
            case 7:
                debug << "BV: transfered from recycler to cashbox"
                      << "\n";
                break;
            default:
                debug << "BV: routing error"
                      << "\n";
                break;
            }
        }
        // Number of input attempts while validator is disabled
        else if (m_outBuffer[i] & 0b01000000)
        {
            // TODO: Implement later
        }
        // Bill recycler only
        else if (m_outBuffer[i] & 0b00100000)
        {
            int val = m_outBuffer[i] & 0b00011111;
            switch (val)
            {
            case 1:
                debug << "BV: escrow request"
                      << "\n";
                break;
            case 2:
                debug << "BV: payout busy"
                      << "\n";
                break;
            case 3:
                debug << "BV: dispenser busy"
                      << "\n";
                break;
            case 4:
                debug << "BV: defective dispenser sensor"
                      << "\n";
                break;
            case 5:
                // Not used
                break;
            case 6:
                debug << "BV: dispenser did not start or motor problem"
                      << "\n";
                break;
            case 7:
                debug << "BV: dispenser jam"
                      << "\n";
                break;
            case 8:
                debug << "BV: ROM checksum error"
                      << "\n";
                break;
            case 9:
                debug << "BV: dispenser disabled"
                      << "\n";
                break;
            case 10:
                debug << "BV: bill waiting"
                      << "\n";
                break;
            case 11: // unused
            case 12: // unused
            case 13: // unused
            case 14: // unused
            case 15:
                debug << "BV: default status"
                      << "\n";
                break;
            default:
                break;
            }
        }
        // Status
        else
        {
            switch (m_outBuffer[i])
            {
            case 1:
                warning << "BV: defective motor"
                        << "\n";
                break;
            case 2:
                warning << "BV: sensor problem"
                        << "\n";
                break;
            case 3:
                warning << "BV: validator busy"
                        << "\n";
                break;
            case 4:
                warning << "BV: ROM checksum error"
                        << "\n";
                break;
            case 5:
                warning << "BV: validator jammed"
                        << "\n";
                break;
            case 6:
                warning << "BV: just reset"
                        << "\n";
                reset = true;
                break;
            case 7:
                warning << "BV: bill removed"
                        << "\n";
                break;
            case 8:
                warning << "BV: cash box out of position"
                        << "\n";
                break;
            case 9:
                warning << "BV: validator disabled"
                        << "\n";
                break;
            case 10:
                warning << "BV: invalid escrow request"
                        << "\n";
                break;
            case 11:
                warning << "BV: bill rejected"
                        << "\n";
                break;
            case 12:
                warning << "BV: possible credited bill removal"
                        << "\n";
                break;
            default:
                break;
            }
        }
    }

    // Just reset
    if (reset && run())
    {
        return MDB_BILL_RESP_JUST_RESET;
    }

    return MDB_BILL_RESP_OK;
}

bool BillValidatorMDB::setup(uint8_t numRetries)
{
    debug << "BV: Init setup"
          << "\n";
    MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS, MDB_CMD_SETUP);
    if (stat != MDBProtocolBase::CMD_STAT_OK)
        return false;

    stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_DEV_BILL_RESP_SIZE_SETUP);

    // Get response containing device info
    if (stat == MDBProtocolBase::CMD_STAT_OK && m_outBufferLen > 0)
    {
        debug << "BV_COM: Received SETUP resp from slave"
              << "\n";
        // Send ACK to slave
        stat = m_pMDB->sendResponseFromMaster(MDBProtocolBase::CMD_STAT_RESP_ACK);
        if (stat == MDBProtocolBase::CMD_STAT_OK)
        {
            // Parse setup info
            m_featureLevel = m_outBuffer[0];
            m_country = m_outBuffer[1] << 8 | m_outBuffer[2];
            m_billScalingFactor = m_outBuffer[3] << 8 | m_outBuffer[4];
            m_decimalPlaces = m_outBuffer[5];
            m_stackerCapacity = m_outBuffer[6] << 8 | m_outBuffer[7];
            m_securityLevels = m_outBuffer[8] << 8 | m_outBuffer[9];
            m_canEscrow = m_outBuffer[10];
            for (int i = 0; i < MDB_DEV_BILL_MAX_TYPE_CREDIT; i++)
            {
                m_billTypeCredit[i] = m_outBuffer[11 + i];
            }

            debug << "BV: Received setup info"
                  << "\n";
            return true;
        }
    }

    // Retries
    if (numRetries < MDB_DEV_MAX_RESET)
    {
        m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
        return setup(++numRetries);
    }

    error << "BV: Setup error"
          << "\n";

    return false;
}

bool BillValidatorMDB::run()
{
    if (!setup())
        return false;

    setSecurity();

    return true;
}

bool BillValidatorMDB::setSecurity(uint8_t numRetries)
{
    uint8_t securityType[] = {0xFF, 0xFF};

    debug << "BV: Init security"
          << "\n";
    m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS, MDB_CMD_BILL_SECURITY, securityType, sizeof(securityType));

    MDBProtocolBase::CmdStatus stat = m_pMDB->getResponseFromSlave(m_outBuffer,
                                                                   &m_outBufferLen,
                                                                   MDB_DEV_BILL_RESP_SIZE_ACK);

    if (stat != MDBProtocolBase::CMD_STAT_RESP_ACK)
    {
        if (numRetries < MDB_DEV_MAX_RESET)
        {
            m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
            return setSecurity(++numRetries);
        }
        warning << "BV: Security failed"
                << "\n";
        return false;
    }

    debug << "BV: Security set completed"
          << "\n";
    return true;
}

bool BillValidatorMDB::setBillType(uint8_t *billType, uint8_t numRetries)
{
    debug << "BV: Set Bill Type"
          << "\n";
    MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS,
                                                                    MDB_CMD_TYPE, billType,
                                                                    MDB_CMD_BILL_TYPE_LEN);
    if (stat != MDBProtocolBase::CMD_STAT_OK)
        return false;

    stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_DEV_BILL_RESP_SIZE_ACK);

    // Get ACK response
    if (stat != MDBProtocolBase::CMD_STAT_RESP_ACK)
    {
        if (numRetries < MDB_DEV_MAX_RESET)
        {
            m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
            return setBillType(billType, ++numRetries);
        }
        warning << "BV: Set Bill Type error"
                << "\n";
        return false;
    }

    debug << "BV: Set Bill Type completed"
          << "\n";
    return true;
}

bool BillValidatorMDB::getStackerStatus(uint8_t numRetries)
{
    debug << "BV: Stacker cmd"
          << "\n";
    MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS,
                                                                    MDB_CMD_BILL_STACKER);
    if (stat != MDBProtocolBase::CMD_STAT_OK)
        return false;

    stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_DEV_BILL_RESP_SIZE_STACKER);

    // Get response containing stacker status and num of bills in the stacker
    if (stat == MDBProtocolBase::CMD_STAT_OK && m_outBufferLen == MDB_DEV_BILL_RESP_SIZE_STACKER)
    {
        // Send ACK to slave
        stat = m_pMDB->sendResponseFromMaster(MDBProtocolBase::CMD_STAT_RESP_ACK);
        if (stat == MDBProtocolBase::CMD_STAT_OK)
        {
            if (m_outBuffer[0] & 0b10000000)
            {
                debug << "BV: Stacker is full"
                      << "\n";
                m_full = true;
            }
            else
            {
                debug << "BV: Stacker is not full"
                      << "\n";
                m_full = false;
            }

            m_billsInStacker = ((uint16_t)(m_outBuffer[0] & 0b01111111) << 8) |
                               (uint16_t)(m_outBuffer[1] & 0xFF);
            debug << "BV: Bills in stacker = " << m_billsInStacker << "\n";

            return true;
        }
    }

    if (numRetries < MDB_DEV_MAX_RESET)
    {
        m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
        return getStackerStatus(++numRetries);
    }

    warning << "BV: Stacker error"
            << "\n";

    return false;
}

bool BillValidatorMDB::escrow(bool acceptBill, uint8_t numRetries)
{
    debug << "BV: Set Escrow"
          << "\n";
    MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS,
                                                                    MDB_CMD_BILL_ESCROW,
                                                                    (uint8_t *)&acceptBill,
                                                                    MDB_CMD_BILL_ESCROW_LEN);
    if (stat != MDBProtocolBase::CMD_STAT_OK)
        return false;

    stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_DEV_BILL_RESP_SIZE_ACK);

    // Get ACK response
    if (stat == MDBProtocolBase::CMD_STAT_RESP_ACK)
    {
        m_billInEscrow = false;
        debug << "BV: Set Escrow completed"
              << "\n";
        return true;
    }

    if (numRetries < MDB_DEV_MAX_RESET)
    {
        m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
        return escrow(acceptBill, ++numRetries);
    }

    error << "BV: Escrow error"
          << "\n";

    return false;
}

/*--------- Sub-cmd implementation ----------*/

bool BillValidatorMDB::scGetLvl1Id(uint8_t numRetries)
{
    debug << "BV: Expansion CMD - Level 1 ID"
          << "\n";
    MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS,
                                                                    MDB_CMD_EXPANSION,
                                                                    MDB_SUBCMD_LVL1_ID);
    if (stat != MDBProtocolBase::CMD_STAT_OK)
        return false;

    stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_SUBCMD_LVL1_ID_LEN);

    // Get response containing level 1 id data
    if (stat == MDBProtocolBase::CMD_STAT_OK && m_outBufferLen == MDB_SUBCMD_LVL1_ID_LEN)
    {
        // Send ACK to slave
        stat = m_pMDB->sendResponseFromMaster(MDBProtocolBase::CMD_STAT_RESP_ACK);
        if (stat == MDBProtocolBase::CMD_STAT_OK)
        {
            uint8_t idx = 0;

            // Manufacturer code
            std::memcpy(m_manufacturerCode, (uint8_t *)&m_outBuffer[idx], MDB_DEV_MANUFAC_LEN);
            idx += MDB_DEV_MANUFAC_LEN;
            // Serial number
            std::memcpy(m_serialNumber, (uint8_t *)&m_outBuffer[idx], MDB_DEV_SER_NO_LEN);
            idx += MDB_DEV_SER_NO_LEN;
            // Model number
            std::memcpy(m_modelNumber, (uint8_t *)&m_outBuffer[idx], MDB_DEV_MOD_NO_LEN);
            idx += MDB_DEV_MOD_NO_LEN;
            // Software version
            std::memcpy(m_softwareVersion, (uint8_t *)&m_outBuffer[idx], MDB_DEV_SW_LEN);
            idx += MDB_DEV_SW_LEN;

            return true;
        }
    }

    if (numRetries < MDB_DEV_MAX_RESET)
    {
        m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
        return scGetLvl1Id(++numRetries);
    }

    warning << "BV: Get Lvl 1 ID failed"
            << "\n";

    return false;
}

bool BillValidatorMDB::scSetLvl2FeatureEnable(uint8_t *featureBytes,
                                              uint8_t len,
                                              uint8_t numRetries)
{
    debug << "BV: Set Level 2+ Feature Enable"
          << "\n";
    if (len != MDB_SUBCMD_LVL2_FEATURE_EN_LEN)
        return false;

    MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS,
                                                                    MDB_CMD_EXPANSION,
                                                                    MDB_SUBCMD_LVL2_FEATURE_EN,
                                                                    featureBytes,
                                                                    MDB_SUBCMD_LVL2_FEATURE_EN_LEN);
    if (stat != MDBProtocolBase::CMD_STAT_OK)
        return false;

    stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_DEV_BILL_RESP_SIZE_ACK);

    // Get ACK response
    if (stat == MDBProtocolBase::CMD_STAT_RESP_ACK)
    {
        debug << "BV: Set Level2+ Feature Enable completed"
              << "\n";
        return true;
    }

    if (numRetries < MDB_DEV_MAX_RESET)
    {
        m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
        return scSetLvl2FeatureEnable(featureBytes, len, ++numRetries);
    }

    warning << "BV: Set Level2+ Feature Enable failed"
            << "\n";

    return false;
}

bool BillValidatorMDB::scGetLvl2Id(uint8_t numRetries)
{
    debug << "BV: Expansion CMD - Level 2 ID"
          << "\n";
    MDBProtocolBase::CmdStatus stat = m_pMDB->sendCommandFromMaster(MDB_DEV_BILL_ADDRESS,
                                                                    MDB_CMD_EXPANSION,
                                                                    MDB_SUBCMD_LVL2_ID);
    if (stat != MDBProtocolBase::CMD_STAT_OK)
        return false;

    stat = m_pMDB->getResponseFromSlave(m_outBuffer, &m_outBufferLen, MDB_SUBCMD_LVL2_ID_LEN);

    // Get response containing level 2 id data
    if (stat == MDBProtocolBase::CMD_STAT_OK && m_outBufferLen == MDB_SUBCMD_LVL2_ID_LEN)
    {
        // Send ACK to slave
        stat = m_pMDB->sendResponseFromMaster(MDBProtocolBase::CMD_STAT_RESP_ACK);
        if (stat == MDBProtocolBase::CMD_STAT_OK)
        {
            uint8_t idx = 0;

            // Manufacturer code
            std::memcpy(m_manufacturerCode, (uint8_t *)&m_outBuffer[idx], MDB_DEV_MANUFAC_LEN);
            idx += MDB_DEV_MANUFAC_LEN;
            // Serial number
            std::memcpy(m_serialNumber, (uint8_t *)&m_outBuffer[idx], MDB_DEV_SER_NO_LEN);
            idx += MDB_DEV_SER_NO_LEN;
            // Model number
            std::memcpy(m_modelNumber, (uint8_t *)&m_outBuffer[idx], MDB_DEV_MOD_NO_LEN);
            idx += MDB_DEV_MOD_NO_LEN;
            // Software version
            std::memcpy(m_softwareVersion, (uint8_t *)&m_outBuffer[idx], MDB_DEV_SW_LEN);
            idx += MDB_DEV_SW_LEN;
            // Optional features
            // -- FTL support
            if (BitHelper::checkBit((uint8_t *)&m_outBuffer[idx++], MDB_DEV_LVL2_OPT_LEN, 0))
                m_ftlSupport = true;
            else
                m_ftlSupport = false;
            // -- Bill Recycling support
            if (BitHelper::checkBit((uint8_t *)&m_outBuffer[idx++], MDB_DEV_LVL2_OPT_LEN, 1))
                m_billRecyclerSupport = true;
            else
                m_billRecyclerSupport = false;
        }
    }

    if (numRetries < MDB_DEV_MAX_RESET)
    {
        m_pTimer->delayMs(MDB_DEV_BILL_DELAY_MS);
        return scGetLvl1Id(++numRetries);
    }

    warning << "BV: Get Lvl 2 ID failed"
            << "\n";

    return false;
}