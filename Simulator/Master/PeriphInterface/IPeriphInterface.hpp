#ifndef IPERIPH_INTERFACE_H
#define IPERIPH_INTERFACE_H

#include <cstdint>
#include <cstring>

struct GenericCallbackConf
{
    void (*cb)(void *arg);
    void *cbArg;
};

class ITimer
{
  public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual uint64_t getTick() = 0;
    virtual uint64_t getPeriodUs() = 0;
    virtual uint64_t getMillis() = 0;
    virtual void delayMs(uint32_t ms) = 0;

    virtual bool enableCallback(uint32_t tick, GenericCallbackConf *cbConf) = 0;
    virtual void disableCallback() = 0;
};

class ISerial
{
  protected:
    static const uint8_t TX_QUEUE_BUFF_MAX = 250;

  public:
    /* Return -1 if error. Otherwise, return the number of item sent*/
    virtual int16_t transmit(uint8_t *data, uint8_t dataLen) = 0;
    /* Return -1 if error. Otherwise, return the number of item received*/
    virtual int16_t receive(uint8_t *buffer, uint8_t bufferLen) = 0;
    /* Queue data for transmission */
    virtual bool queue(uint8_t *data, uint8_t dataLen) = 0;
    /* Start queue transmission */
    virtual int16_t transmitQueue() = 0;
    /* Clear queue */
    virtual void clearQueue() = 0;
    /* Set bit 9 mode */
    virtual void setModeBit(bool modeBit) = 0;
    /* Get bit 9 mode */
    virtual bool getModeBit() = 0;
    /* Check if RX is available */
    virtual bool hasData() = 0;

  protected:
    uint8_t m_txQueue[TX_QUEUE_BUFF_MAX];
    uint8_t idxQueue;
};

#endif
