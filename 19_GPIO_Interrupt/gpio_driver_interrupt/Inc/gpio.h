#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <stdint.h>

/* ================= BASE ADDRESSES ================= */
#define PERIPH_BASE        0x40000000UL
#define AHB1_BASE          (PERIPH_BASE + 0x00020000UL)
#define APB2_BASE          (PERIPH_BASE + 0x00010000UL)

#define GPIOA_BASE         (AHB1_BASE + 0x0000)
#define GPIOC_BASE         (AHB1_BASE + 0x0800)
#define RCC_BASE           (AHB1_BASE + 0x3800)
#define SYSCFG_BASE        (APB2_BASE + 0x3800)
#define EXTI_BASE          (APB2_BASE + 0x3C00)

/* ================= STRUCTURES ================= */
typedef struct
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
} GPIO_TypeDef;

#define GPIOA   ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)

#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB2ENR   (*(volatile uint32_t *)(RCC_BASE + 0x44))

#define SYSCFG_EXTICR4  (*(volatile uint32_t *)(SYSCFG_BASE + 0x14))

#define EXTI_IMR     (*(volatile uint32_t *)(EXTI_BASE + 0x00))
#define EXTI_FTSR    (*(volatile uint32_t *)(EXTI_BASE + 0x0C))
#define EXTI_PR      (*(volatile uint32_t *)(EXTI_BASE + 0x14))

#define NVIC_ISER1   (*(volatile uint32_t *)(0xE000E104))

/* ================= APIs ================= */
void GPIO_EnableClock(GPIO_TypeDef *GPIOx);
void GPIO_ConfigOutput(GPIO_TypeDef *GPIOx, uint8_t pin);
void GPIO_ConfigInput(GPIO_TypeDef *GPIOx, uint8_t pin);
void GPIO_Toggle(GPIO_TypeDef *GPIOx, uint8_t pin);

void EXTI_Config_PC13(void);

#endif
