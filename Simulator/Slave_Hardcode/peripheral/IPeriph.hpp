#ifndef PERIPH_INTERFACE_H
#define PERIPH_INTERFACE_H

#include <stdint.h>
#include <string.h>

struct GenericCallbackConf
{
    void (*cb)(void *arg);
    void *cbArg;
};

class IPwmOutput
{
  public:
    virtual void setValue(uint32_t value) = 0;
    virtual uint32_t getValue() = 0;
    virtual uint32_t getMaxValue() = 0;

    virtual uint32_t getPeriodUs() = 0;
    virtual void setPeriod(uint64_t periodUs) = 0;
};

class IGpio
{
  public:
    enum EdgeType
    {
        RISING_EDGE,
        FALLING_EDGE,
        BOTH_EDGE
    };

    virtual void set() = 0;
    virtual void clear() = 0;
    virtual void write(bool active) = 0;
    virtual void toggle() = 0;
    virtual bool get() = 0;

    virtual void enableEdgeCallback(EdgeType edge, GenericCallbackConf *pCbConf) = 0;
    virtual void disableEdgeCallback() = 0;
};

class IAdc
{
  public:
    virtual uint32_t get() = 0;
    virtual uint32_t maxVal() = 0;
};

class IUart
{
  public:
    enum CmdStatus
    {
        CMD_SUCCESS = 0,
        CMD_UART_BUSY = -1,
        CMD_UART_ERROR = -2
    };

    struct TxCallbackConf
    {
        void (*cb)(bool isError, void *arg);
        void *cbArg;
    };

    struct RxCallbackConf
    {
        void (*cb)(bool isError, uint8_t *rxData, uint32_t rxDataLen, void *arg);
        void *cbArg;
    };

    virtual CmdStatus transmit(uint8_t *txData, uint32_t txDataLen) = 0;
    virtual CmdStatus receive(uint8_t *rxBuffer, uint32_t rxBufferSize, uint32_t *pRxDataLen) = 0; //specify the fifo size in init
    virtual uint32_t getDataCount() = 0;
    virtual uint16_t getErrorCode() = 0;

    virtual CmdStatus transmitAsync(uint8_t *txData, uint32_t txDataLen, TxCallbackConf *pFinishedCbConf) = 0;
    virtual CmdStatus startReceiveAsync(RxCallbackConf *pReceivedCbConf) = 0;
    virtual CmdStatus stopReceiveAsync() = 0;
};

class ITimer
{
  public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual uint64_t getTick() = 0;
    virtual uint64_t getPeriodUs() = 0;
    virtual uint64_t getMillis() = 0;

    virtual bool enableCallback(uint32_t tick, GenericCallbackConf *cbConf) = 0;
    virtual void disableCallback() = 0;
};

class IWatchdog
{
  public:
    virtual void Kick() = 0;
};

class IInputCapture
{
  public:
    enum EdgeType
    {
        RISING_EDGE,
        FALLING_EDGE,
        BOTH_EDGE
    };

    virtual void startCapture(EdgeType edge, GenericCallbackConf *cbConf = NULL) = 0;
    virtual bool isFinished() = 0;
    virtual uint32_t getTimingUs() = 0;
};

class IPwmInput
{
  public:
    virtual void startMeasure(GenericCallbackConf *cbConf = NULL) = 0;
    virtual bool isFinished() = 0;
    virtual uint32_t getPulseWidthUs() = 0;
    virtual uint32_t getPulsePeriodUs() = 0;
};

class ISignalCounter
{
  public:
    virtual void startCounting() = 0;
    virtual void pauseCounting() = 0;
    virtual void clearCount() = 0;
    virtual uint32_t getCount() = 0;
};

class IDac
{
  public:
    virtual void setValue(uint32_t value) = 0;
    virtual uint32_t getValue() = 0;
    virtual uint32_t maxValue() = 0;
};

class ISpi
{
  public:
    virtual void transmit(uint8_t *dataOut, uint32_t dataLen) = 0;
    virtual uint32_t receive(uint8_t *dataIn, uint32_t maxDataLen) = 0;
    virtual uint32_t transceive(uint8_t *dataOut, uint8_t *dataIn, uint32_t maxDataLen) = 0;

    virtual bool acquire() = 0;
    virtual void release() = 0;

    // in consideration, in transmit receive or not
    virtual uint8_t isError() = 0;
};

class II2c
{
  public:
    virtual void start() = 0;
    virtual void write(uint16_t devAddr, uint16_t regAddr, uint8_t *dataOut, uint32_t dataLen) = 0;
    virtual uint32_t read(uint8_t *dataIn, uint32_t maxDataLen) = 0;
    virtual void stop() = 0;

    // in consideration, in transmit receive or not
    virtual uint8_t isError() = 0;
};

class IEncoder
{
  public:
    virtual uint32_t getCount() = 0;
    virtual void resetCount() = 0;
    virtual uint8_t isError() = 0;
};

#endif