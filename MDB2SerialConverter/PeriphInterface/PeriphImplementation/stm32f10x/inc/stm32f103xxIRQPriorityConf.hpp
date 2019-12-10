#ifndef STM32F103XX_IRQ_PRIORITY_CONF_H
#define STM32F103XX_IRQ_PRIORITY_CONF_H

/* There are only 0-15 IRQ Priority
 * Note that, if you use FreeRTOS, the default available priority is only 5-14
 * However, it is legal to set same priority for various IRQ
 */

#define USART_IRQN_PRIORITY     6

#endif