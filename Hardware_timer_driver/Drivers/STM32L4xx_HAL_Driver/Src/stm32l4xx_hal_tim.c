/*
 * stm32l4xx_hal_tim.c
 *
 *  Created on: Mar 31, 2026
 *      Author: yekol
 */


#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx.h"


/* Basic Init */
uint32_t HAL_TIM_Base_Init(TIM_HandleTypeDef *htim)
{
    TIM_TypeDef *TIMx = (TIM_TypeDef *)htim->Instance;

    TIMx->PSC = htim->Init.Prescaler;
    TIMx->ARR = htim->Init.Period;
    TIMx->CNT = 0;

    return HAL_OK;
}

/* Start */
uint32_t HAL_TIM_Base_Start(TIM_HandleTypeDef *htim)
{
    TIM_TypeDef *TIMx = (TIM_TypeDef *)htim->Instance;
    TIMx->CR1 |= 1;  // CEN
    return HAL_OK;
}

/* Stop */
uint32_t HAL_TIM_Base_Stop(TIM_HandleTypeDef *htim)
{
    TIM_TypeDef *TIMx = (TIM_TypeDef *)htim->Instance;
    TIMx->CR1 &= ~1;
    return HAL_OK;
}

/* Start with interrupt */
uint32_t HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim)
{
    TIM_TypeDef *TIMx = (TIM_TypeDef *)htim->Instance;
    TIMx->DIER |= 1;  // Update interrupt enable
    TIMx->CR1 |= 1;
    return HAL_OK;
}

/* Stop interrupt */
uint32_t HAL_TIM_Base_Stop_IT(TIM_HandleTypeDef *htim)
{
    TIM_TypeDef *TIMx = (TIM_TypeDef *)htim->Instance;
    TIMx->DIER &= ~1;
    TIMx->CR1 &= ~1;
    return HAL_OK;
}

/* IRQ handler */
void HAL_TIM_IRQHandler(TIM_HandleTypeDef *htim)
{
    TIM_TypeDef *TIMx = (TIM_TypeDef *)htim->Instance;

    if (TIMx->SR & 1)
    {
        TIMx->SR &= ~1;
        HAL_TIM_PeriodElapsedCallback(htim);
    }
}

/* Weak callback */
__attribute__((weak)) void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* user override */
}
