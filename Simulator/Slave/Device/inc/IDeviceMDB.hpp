#ifndef IDEVICE_MDB_H
#define IDEVICE_MDB_H

#include <MDBProtocolBase.hpp>
#include <Logger.hpp>

class IDeviceMDB
{
  protected:
    static const uint8_t MDB_DEV_MAX_RESET = 5;
    static const uint8_t MDB_DEV_MAX_RESET_POLL = 15;
    static const uint16_t MDB_DEV_SETUP_TIME = 200;
    static const uint32_t MDB_DEV_RESPONSE_TIME = 5;

    static const uint8_t MDB_DEV_SER_NO_LEN = 12;
    static const uint8_t MDB_DEV_MOD_NO_LEN = 12;
    static const uint8_t MDB_DEV_MANUFAC_LEN = 3;
    static const uint8_t MDB_DEV_SW_LEN = 2;

    static const uint8_t MDB_OUTBUFFER_LEN = 72;

  public:
    enum CmdMDB
    {
        MDB_CMD_RESET = 0x00,
        MDB_CMD_SETUP = 0x01,
        MDB_CMD_POLL = 0x03,
        MDB_CMD_TYPE = 0x04,
        MDB_CMD_EXPANSION = 0x07
    };
    virtual void initialize(MDBProtocolBase *mdb, ISerial *pCom, ITimer *pTimer) = 0;
    virtual bool reset() = 0;
    virtual void print() = 0;

  protected:
    virtual int16_t poll() = 0;

    MDBProtocolBase *m_pMDB;
    ISerial *m_pSerial;
    ITimer *m_pTimer;

    uint16_t m_resetCount;
    uint16_t m_count;

    uint8_t m_featureLevel;
    uint16_t m_country;
    uint8_t *m_manufacturerCode;
    uint8_t *m_serialNumber;
    uint8_t *m_modelNumber;
    uint8_t *m_softwareVersion;

    uint8_t m_outBuffer[MDB_OUTBUFFER_LEN];
    uint8_t m_outBufferLen;
};

#endif
