#ifndef __STM32L4xx_HAL_TIM_H
#define __STM32L4xx_HAL_TIM_H

#include "stm32l4xx_hal_def.h"

/* TIM Base handle */
typedef struct
{
    void *Instance;

    struct
    {
        uint32_t Prescaler;
        uint32_t CounterMode;
        uint32_t Period;
        uint32_t ClockDivision;
        uint32_t AutoReloadPreload;
    } Init;

} TIM_HandleTypeDef;

/* Counter modes */
#define TIM_COUNTERMODE_UP           0x00000000U

/* Clock division */
#define TIM_CLOCKDIVISION_DIV1       0x00000000U

/* Auto reload preload */
#define TIM_AUTORELOAD_PRELOAD_DISABLE   0x00000000U

/* HAL status */
#define HAL_OK 0x00U

/* Function prototypes */
uint32_t HAL_TIM_Base_Init(TIM_HandleTypeDef *htim);
uint32_t HAL_TIM_Base_Start(TIM_HandleTypeDef *htim);
uint32_t HAL_TIM_Base_Stop(TIM_HandleTypeDef *htim);
uint32_t HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim);
uint32_t HAL_TIM_Base_Stop_IT(TIM_HandleTypeDef *htim);
void HAL_TIM_IRQHandler(TIM_HandleTypeDef *htim);

#endif
