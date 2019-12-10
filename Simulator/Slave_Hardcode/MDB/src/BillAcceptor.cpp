#include "../inc/BillAcceptor.hpp"
#include <string.h>

using namespace Linux;

const uint8_t BillAcceptor::COUNTRY_CODE[2] = {0x13, 0x60};
const uint8_t BillAcceptor::BILL_SCALING[2] = {0x03, 0xE8};

const uint8_t BillAcceptor::STACKCER_CAPACITY[2] = {0x01, 0x90};
const uint8_t BillAcceptor::BILL_TYPE[16] = {0x01, 0x02, 0x05, 0x0A, 0x14, 0x032, 0x64};
const uint8_t BillAcceptor::SECURITY_LEVEL[2] = {0x00, 0x02};

const uint8_t BillAcceptor::MANUFACTURER_CODE[4] = "JCM";
const uint8_t BillAcceptor::SW_VERSION[2] = {0x45, 0x13};

const uint8_t BillAcceptor::m_serial_number[13] = "244466666888";
const uint8_t BillAcceptor::m_model_number[13] = "113333555555";
const uint8_t BillAcceptor::JUST_RESET_TX[11] = "JUST RESET";

BillAcceptor::BillAcceptor(Serial *mdb, Linux::MillisTimer *milis)
{
    m_mdb = mdb;
    m_milis = milis;
}

void BillAcceptor::listen()
{
    uint8_t buff[32] = {0};
    uint32_t rxLength = 0;
    uint8_t checksum = 0;

    if (m_mdb->receive(buff, sizeof(buff), &rxLength) > -1)
    {
        if (rxLength > 1)
        {
            for (uint8_t i = 0; i < rxLength - 1; i++)
            {
                checksum += buff[i];
            }

            uint8_t addr = buff[0] & 0xF8;
            uint8_t cmd = buff[0] & 0x07;

            if (addr == ADDRESS && checksum == buff[rxLength - 1])
            {
                switch (cmd)
                {
                case 0:
                    m_reset = Reset();
                    break;
                case 1:
                    setup();
                    break;
                case 2:
                    if (rxLength == 4)
                    {
                        security(buff);
                    }
                    break;
                case 3:
                    poll();
                    break;
                case 4:
                    if (rxLength == 6)
                    {
                        billType(buff);
                    }
                    break;
                case 5:
                    if (rxLength == 3)
                    {
                        escrow(buff);
                    }
                    break;
                case 6:
                    stacker();
                    break;
                case 7:
                    if (rxLength == 3 && buff[1] == IDENTIFICATION_DATA)
                    {
                        identification();
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
}

bool BillAcceptor::Reset()
{
    if (m_reset) {
        m_mdb->transmit(&ACK, sizeof(ACK));
    }
    
    return RESET_SUCCESS;
}

int BillAcceptor::poll()
{
    if (m_reset)
    {
        m_mdb->transmit((uint8_t *)JUST_RESET_TX, sizeof(JUST_RESET_TX) - 1);
        m_reset = false;
        return 0;
    }

    return 0;
}

void BillAcceptor::setup()
{
    m_checksum = 0;

    m_mdb->transmit(&FEATURE_LEVEL, sizeof(FEATURE_LEVEL));

    m_checksum += FEATURE_LEVEL;

    m_mdb->transmit((uint8_t *)COUNTRY_CODE, sizeof(COUNTRY_CODE));

    for (uint8_t i = 0; i < sizeof(COUNTRY_CODE); i++)
    {
        m_checksum += COUNTRY_CODE[i];
    }

    m_mdb->transmit((uint8_t *)BILL_SCALING, sizeof(BILL_SCALING));

    for (uint8_t i = 0; i < sizeof(BILL_SCALING); i++)
    {
        m_checksum += BILL_SCALING[i];
    }

    m_mdb->transmit(&DECIMAL, sizeof(DECIMAL));

    m_checksum += DECIMAL;

    m_mdb->transmit((uint8_t *)STACKCER_CAPACITY, sizeof(STACKCER_CAPACITY));

    for (uint8_t i = 0; i < sizeof(STACKCER_CAPACITY); i++)
    {
        m_checksum += STACKCER_CAPACITY[i];
    }

    m_mdb->transmit((uint8_t *)SECURITY_LEVEL, sizeof(SECURITY_LEVEL));

    for (uint8_t i = 0; i < sizeof(SECURITY_LEVEL); i++)
    {
        m_checksum += SECURITY_LEVEL[i];
    }

    m_mdb->transmit(&ESCROW, sizeof(ESCROW));

    m_checksum += ESCROW;

    m_mdb->transmit((uint8_t *)BILL_TYPE, sizeof(BILL_TYPE));

    for (uint8_t i = 0; i < sizeof(BILL_TYPE); i++)
    {
        m_checksum += BILL_TYPE[i];
    }

    m_mdb->transmit(&m_checksum, sizeof(m_checksum));
}

void BillAcceptor::Print()
{
}

void BillAcceptor::identification()
{
    m_checksum = 0;

    m_mdb->transmit((uint8_t *)MANUFACTURER_CODE, sizeof(MANUFACTURER_CODE) - 1);

    for (uint8_t i = 0; i < sizeof(MANUFACTURER_CODE); i++)
    {
        m_checksum += MANUFACTURER_CODE[i];
    }

    m_mdb->transmit((uint8_t *)m_serial_number, sizeof(m_serial_number) - 1);

    for (uint8_t i = 0; i < sizeof(m_serial_number); i++)
    {
        m_checksum += m_serial_number[i];
    }

    m_mdb->transmit((uint8_t *)m_model_number, sizeof(m_model_number) - 1);

    for (uint8_t i = 0; i < sizeof(m_model_number); i++)
    {
        m_checksum += m_model_number[i];
    }

    m_mdb->transmit((uint8_t *)SW_VERSION, sizeof(SW_VERSION));

    for (uint8_t i = 0; i < sizeof(SW_VERSION); i++)
    {
        m_checksum += SW_VERSION[i];
    }

    m_mdb->transmit(&m_checksum, sizeof(m_checksum));
}

void BillAcceptor::stacker()
{
    m_mdb->transmit((uint8_t *)m_stackerTx, sizeof(m_stackerTx));

    for (uint8_t i = 0; i < sizeof(m_stackerTx); i++)
    {
        m_checksum += m_stackerTx[i];
    }

    m_mdb->transmit(&m_checksum, sizeof(m_checksum));
}

void BillAcceptor::billType(uint8_t *buff)
{
    m_billEnable = ((m_billEnable | buff[1]) << 8) | buff[2];
    m_escrowEnable = ((m_escrowEnable | buff[3]) << 8) | buff[4];

    m_mdb->transmit(&ACK, sizeof(ACK));
}

void BillAcceptor::security(uint8_t *buff)
{
    m_securityEnable = ((m_securityEnable | buff[1]) << 8) | buff[2];

    m_mdb->transmit(&ACK, sizeof(ACK));
}

void BillAcceptor::escrow(uint8_t *buff)
{
    m_escrow = buff[1];

    m_mdb->transmit(&ACK, sizeof(ACK));
}

void BillAcceptor::waitingAck()
{
}
