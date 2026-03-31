#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/* ================= BASE ADDRESSES ================= */
#define PERIPH_BASE        0x40000000UL
#define AHB1_OFFSET        0x00020000UL
#define AHB1PERIPH_BASE    (PERIPH_BASE + AHB1_OFFSET)

#define GPIOA_BASE         (AHB1PERIPH_BASE + 0x0000)
#define GPIOB_BASE         (AHB1PERIPH_BASE + 0x0400)
#define GPIOC_BASE         (AHB1PERIPH_BASE + 0x0800)

#define RCC_BASE           (PERIPH_BASE + 0x23800)

/* ================= REGISTER STRUCTURE ================= */
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

/* ================= POINTERS ================= */
#define GPIOA   ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))

/* ================= MODES ================= */
#define GPIO_INPUT      0x00
#define GPIO_OUTPUT     0x01
#define GPIO_ALT        0x02
#define GPIO_ANALOG     0x03

/* ================= FUNCTION PROTOTYPES ================= */
void GPIO_EnableClock(GPIO_TypeDef *GPIOx);
void GPIO_SetMode(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t mode);
void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state);
void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint8_t pin);
uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint8_t pin);

#endif
