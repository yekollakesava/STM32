#ifndef __STM32L4xx_HAL_CONF_H
#define __STM32L4xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* HAL MODULES */
#define HAL_MODULE_ENABLED

#define HAL_RCC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

/* CLOCK VALUES */
#define HSE_VALUE                    8000000U
#define HSI_VALUE                    16000000U
#define MSI_VALUE                    4000000U
#define LSI_VALUE                    32000U
#define LSE_VALUE                    32768U

#define VDD_VALUE                    3300U
#define TICK_INT_PRIORITY            0U

/* IMPORTANT: DO NOT CHANGE ORDER */
#include "stm32l4xx_hal.h"   // ✅ THIS FIXES YOUR ERROR

#ifdef __cplusplus
}
#endif

#endif
