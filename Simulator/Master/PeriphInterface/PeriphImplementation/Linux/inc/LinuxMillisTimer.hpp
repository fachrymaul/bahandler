#ifndef LINUX_MILLIS_TIMER_H
#define LINUX_MILLIS_TIMER_H

#include <IPeriphInterface.hpp>
#include <time.h>

namespace Linux
{
class MillisTimer final : public ITimer
{
  public:
    MillisTimer();
    ~MillisTimer();

    virtual void start();
    virtual void stop();
    virtual uint64_t getTick();
    virtual uint64_t getPeriodUs();
    virtual uint64_t getMillis();
    virtual void delayMs(uint32_t ms);

    virtual bool enableCallback(uint32_t tick, GenericCallbackConf *cbConf);
    virtual void disableCallback();

    bool isError();

  private:
    struct timespec m_now;
    bool m_isStarted;
    bool m_isError;
};
} // namespace Linux

#endif
