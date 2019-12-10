#include <LinuxMillisTimer.hpp>
#include <math.h>

using namespace Linux;

MillisTimer::MillisTimer()
{
    memset(&m_now, 0, sizeof(m_now));
    m_isStarted = false;
    m_isError = false;
}

MillisTimer::~MillisTimer()
{
}

bool MillisTimer::isError()
{
    return m_isError;
}

void MillisTimer::start()
{
    m_isStarted = true;
}

void MillisTimer::stop()
{
    m_isStarted = false;
}

uint64_t MillisTimer::getTick()
{
    if (m_isStarted)
    {
        m_isError = clock_gettime(CLOCK_MONOTONIC, &m_now) < 0 ? true : false;
    }

    return (m_now.tv_sec * 1.0e3) + (m_now.tv_nsec * 1.0e-6); // * 1.0e3 + m_now.tv_nsec * 1.0e-6;
}

uint64_t MillisTimer::getPeriodUs()
{
    // TODO: Implement later
    return 1000;
}

uint64_t MillisTimer::getMillis()
{
    return getTick();
}

void MillisTimer::delayMs(uint32_t ms)
{
    uint64_t startMillis = getTick();
    while (getTick() - startMillis <= ms)
        ;
}

bool MillisTimer::enableCallback(uint32_t tick, GenericCallbackConf *cbConf)
{
    // TODO: implement later
    return false;
}

void MillisTimer::disableCallback()
{
    // TODO: implement later
}
