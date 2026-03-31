#include "gpio.h"

/* Enable GPIO Clock */
void GPIO_EnableClock(GPIO_TypeDef *GPIOx)
{
    if(GPIOx == GPIOA)
        RCC_AHB1ENR |= (1 << 0);
    else if(GPIOx == GPIOC)
        RCC_AHB1ENR |= (1 << 2);
}

/* Configure Output */
void GPIO_ConfigOutput(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    GPIOx->MODER &= ~(3 << (2 * pin));
    GPIOx->MODER |=  (1 << (2 * pin));
}

/* Configure Input */
void GPIO_ConfigInput(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    GPIOx->MODER &= ~(3 << (2 * pin));
}

/* Toggle Pin */
void GPIO_Toggle(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    GPIOx->ODR ^= (1 << pin);
}

/* Configure EXTI for PC13 */
void EXTI_Config_PC13(void)
{
    /* Enable SYSCFG clock */
    RCC_APB2ENR |= (1 << 14);

    /* Map PC13 to EXTI13 */
    SYSCFG_EXTICR4 &= ~(0xF << 4);
    SYSCFG_EXTICR4 |=  (0x2 << 4);

    /* Unmask interrupt */
    EXTI_IMR |= (1 << 13);

    /* Falling edge trigger */
    EXTI_FTSR |= (1 << 13);

    /* Enable NVIC (IRQ40) */
    NVIC_ISER1 |= (1 << 8);
}
