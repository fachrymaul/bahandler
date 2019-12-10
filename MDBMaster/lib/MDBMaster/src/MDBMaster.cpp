/**
 * @file MDBMaster.cpp
 * @brief Implementation of MDBMaster.hpp
 *
 * @copyright All Rights Reserved to Bukalapak
 * @author Bobby P. Johan
 * @author Richard
 */
#include <../../MDBMaster/inc/MDBMaster.hpp>


static const uint8_t setupData[5]       = {0xFE, 0x04, 0x01, 0xE8, 0x2A};
static const uint8_t resetData[5]       = {0xFE, 0x04, 0x00, 0xF8, 0x0B};
static const uint8_t acceptMoneyData[5] = {0xFE, 0x04, 0x71, 0x96, 0xBD};

static const uint8_t enableBillType1k[5]   = {0xFE, 0x04, 0x31, 0xDE, 0x79};
static const uint8_t enableBillType2k[5]   = {0xFE, 0x04, 0x32, 0xEE, 0x1A};
static const uint8_t enableBillType5k[5]   = {0xFE, 0x04, 0x33, 0xFE, 0x3B};
static const uint8_t enableBillType10k[5]  = {0xFE, 0x04, 0x34, 0x8E, 0xDC};
static const uint8_t enableBillType20k[5]  = {0xFE, 0x04, 0x35, 0x9E, 0xFD};
static const uint8_t enableBillType50k[5]  = {0xFE, 0x04, 0x36, 0xAE, 0x9E};
static const uint8_t enableBillType100k[5] = {0xFE, 0x04, 0x37, 0xBE, 0xBF};
static const uint8_t enableBillTypeAll[5]  = {0xFE, 0x04, 0x38, 0x4F, 0x50};

static const uint8_t disableBillType1k[5]   = {0xFE, 0x04, 0x39, 0x5F, 0x71};
static const uint8_t disableBillType2k[5]   = {0xFE, 0x04, 0x3A, 0x6F, 0x12};
static const uint8_t disableBillType5k[5]   = {0xFE, 0x04, 0x3B, 0x7F, 0x33};
static const uint8_t disableBillType10k[5]  = {0xFE, 0x04, 0x3C, 0xFD, 0x04};
static const uint8_t disableBillType20k[5]  = {0xFE, 0x04, 0x3D, 0x1F, 0xF5};
static const uint8_t disableBillType50k[5]  = {0xFE, 0x04, 0x3E, 0x2F, 0x96};
static const uint8_t disableBillType100k[5] = {0xFE, 0x04, 0x3F, 0x3F, 0xB7};
static const uint8_t disableBillTypeAll[5]  = {0xFE, 0x04, 0x40, 0xB0, 0xCF};

static const uint8_t getInfoData[5] = {0xFE, 0x04, 0x81, 0x79, 0xA2};

MDBMaster::MDBMaster()
{
    m_serialProxy = nullptr;
    m_billType    = 0;
    
    m_mdbInfo.manfCode       = 0;
    m_mdbInfo.countryCode    = 0;
    m_mdbInfo.stackedBill    = 0;
    m_mdbInfo.activeBillType = 0;
    m_mdbInfo.statusCode     = 0;
}

uint8_t MDBMaster::acceptMoney()
{
    sendData(acceptMoneyData, sizeof(acceptMoneyData));
    return true;
}

bool MDBMaster::sendData(const uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        m_serialProxy->write(data[i]);
    }
    return true;
}

bool MDBMaster::setup(ISerial *serialProxy)
{
    m_serialProxy = serialProxy;
    // TODO setup
    sendData(setupData, sizeof(setupData));
    return true;
}

bool MDBMaster::reset()
{
    sendData(resetData, sizeof(resetData));
    return true;
}

MDBMaster::MdbInfo MDBMaster::getInfo()
{
    sendData(getInfoData, sizeof(getInfoData));
    // TODO parse return value of getInfo
    return info;
}

bool MDBMaster::enableBillType(M2P::BillTypeSwitch billType)
{
    // TODO activeBillType |= billTypeSwitch(billType)
    switch (billType)
    {
    case M2P::SW_BT_1K:
        m_billType |= billType;
        return sendData(enableBillType1k, sizeof(enableBillType1k));

    case M2P::SW_BT_2K:
        m_billType |= billType;
        return sendData(enableBillType2k, sizeof(enableBillType2k));

    case M2P::SW_BT_5K:
        m_billType |= billType;
        return sendData(enableBillType5k, sizeof(enableBillType5k));

    case M2P::SW_BT_10K:
        m_billType |= billType;
        return sendData(enableBillType10k, sizeof(enableBillType10k));

    case M2P::SW_BT_20K:
        m_billType |= billType;
        return sendData(enableBillType20k, sizeof(enableBillType20k));

    case M2P::SW_BT_50K:
        m_billType |= billType;
        return sendData(enableBillType50k, sizeof(enableBillType50k));

    case M2P::SW_BT_100K:
        m_billType |= billType;
        return sendData(enableBillType100k, sizeof(enableBillType100k));

    case M2P::SW_BT_ENA_ALL:
        m_billType |= billType;
        return sendData(enableBillTypeAll, sizeof(enableBillTypeAll));

    default:
        return false;
    }

    // TODO billTypeCmd(activeBillType)
}

/**
 * Disable Bill Type want to insert to bill acceptor
 * @param data to enable money 1k - 100k
 * @return true for success execute
 * @return false for failed execute
 */
bool MDBMaster::disBillType(M2P::BillTypeSwitch billType)
{
    // TODO activeBillType |= billTypeSwitch(billType)
    switch (billType)
    {
    case M2P::SW_BT_1K:
        m_billType &= ~billType;
        return sendData(disableBillType1k, sizeof(disableBillType1k));

    case M2P::SW_BT_2K:
        m_billType &= ~billType;
        return sendData(disableBillType2k, sizeof(disableBillType2k));

    case M2P::SW_BT_5K:
        m_billType &= ~billType;
        return sendData(disableBillType5k, sizeof(disableBillType5k));

    case M2P::SW_BT_10K:
        m_billType &= ~billType;
        return sendData(disableBillType10k, sizeof(disableBillType10k));

    case M2P::SW_BT_20K:
        m_billType &= ~billType;
        return sendData(disableBillType20k, sizeof(disableBillType20k));

    case M2P::SW_BT_50K:
        m_billType &= ~billType;
        return sendData(disableBillType50k, sizeof(disableBillType50k));

    case M2P::SW_BT_100K:
        m_billType &= ~billType;
        return sendData(disableBillType100k, sizeof(disableBillType100k));

    case M2P::SW_BT_ENA_ALL:
        m_billType &= ~billType;
        return sendData(disableBillTypeAll, sizeof(disableBillTypeAll));

    default:
        return false;
    }
}

bool MDBMaster::resetBillType()
{
    bool status = false;
    if (sendData(disableBillTypeAll, sizeof(disableBillTypeAll)))
    {
        m_billType = 0;
        status = true;
    }
    return status;
}

bool MDBMaster::resetMoney()
{
    m_money = 0;
    return true;
}
