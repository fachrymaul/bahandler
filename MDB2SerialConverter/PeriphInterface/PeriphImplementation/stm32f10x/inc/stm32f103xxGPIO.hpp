#ifndef STM32F103XX_GPIO_H
#define STM32F103XX_GPIO_H

#include <IPeriphInterface.hpp>
#include <stm32f10x.h>

namespace stm32f103xx
{
class Gpio final : public IGpio
{
public:
    Gpio();
    ~Gpio();

    void initialize(GPIO_TypeDef* gpioPort, uint16_t gpioPin, bool isOutput = false);
    
    virtual void set();
    virtual void clear();
    virtual void write(bool active);
    virtual void toggle();
    virtual bool get();

    virtual void enableEdgeCallback(EdgeType edge, GenericCallbackConf* pCbConf);
    virtual void disableEdgeCallback();

private:
    GPIO_TypeDef* m_gpioPort;
    uint16_t m_gpioPin;
    bool m_isOutput;
};
}

#endif