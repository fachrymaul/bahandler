#include <stm32f103xxGPIO.hpp>

using namespace stm32f103xx;

Gpio::Gpio()
{
    m_gpioPort = NULL;
    m_gpioPin = 0;
    m_isOutput = false;
}

Gpio::~Gpio()
{
}

void Gpio::initialize(GPIO_TypeDef* gpioPort, uint16_t gpioPin, bool isOutput)
{
    m_gpioPort = gpioPort;
    m_gpioPin = gpioPin;
    m_isOutput = isOutput;
}

void Gpio::set()
{
    // TODO: Implement for F103
    m_gpioPort->BSRR = 0x01 << m_gpioPin;
}

void Gpio::clear()
{
    // TODO: Implement for F103
    m_gpioPort->BSRR = 0x01 << m_gpioPin;
}

void Gpio::write(bool active)
{
    active ? set() : clear();
}

void Gpio::toggle()
{
	m_gpioPort->ODR ^= (0x01 << m_gpioPin);
}

bool Gpio::get()
{
    return (((m_isOutput ? m_gpioPort->ODR : m_gpioPort->IDR) >> m_gpioPin) & 0x01) > 0x00;
}

void Gpio::enableEdgeCallback(EdgeType edge, GenericCallbackConf* pCbConf)
{
    //TODO : implement this later
}

void Gpio::disableEdgeCallback()
{
    //TODO : implement this later   
}