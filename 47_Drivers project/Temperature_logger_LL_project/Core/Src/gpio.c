#include "stm32l476xx.h"
#include "gpio.h"

void GPIO_Init(void)
{
    /* Enable GPIO clocks */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    /* ================= UART2 (PA2, PA3) ================= */
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |=  (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);

    GPIOA->AFR[0] &= ~((0xF << (2*4)) | (0xF << (3*4)));
    GPIOA->AFR[0] |=  (7 << (2*4)) | (7 << (3*4));   // AF7 = USART2

    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD3);
    GPIOA->PUPDR |=  GPIO_PUPDR_PUPD3_0;  // Pull-up RX

    /* ================= SPI1 (PB3,4,5) ================= */
    GPIOB->MODER &= ~(GPIO_MODER_MODE3 |
                      GPIO_MODER_MODE4 |
                      GPIO_MODER_MODE5);

    GPIOB->MODER |=  (GPIO_MODER_MODE3_1 |
                      GPIO_MODER_MODE4_1 |
                      GPIO_MODER_MODE5_1);

    GPIOB->AFR[0] &= ~((0xF << (3*4)) |
                       (0xF << (4*4)) |
                       (0xF << (5*4)));

    GPIOB->AFR[0] |=  (5 << (3*4)) |
                      (5 << (4*4)) |
                      (5 << (5*4));

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD4);
    GPIOB->PUPDR |=  GPIO_PUPDR_PUPD4_0;  // MISO pull-up

    /* ================= CS (PB8) ================= */
    GPIOB->MODER &= ~(GPIO_MODER_MODE8);
    GPIOB->MODER |=  GPIO_MODER_MODE8_0;

    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT8);
    GPIOB->OSPEEDR |= (3 << (8*2));
    GPIOB->ODR |= GPIO_ODR_OD8;

    /* ================= I2C1 (PB6, PB7) ================= */

    // PB6 SCL
    GPIOB->MODER &= ~(3 << (6*2));
    GPIOB->MODER |=  (2 << (6*2));
    GPIOB->OTYPER |= (1 << 6);
    GPIOB->OSPEEDR |= (3 << (6*2));
    GPIOB->PUPDR &= ~(3 << (6*2));
    GPIOB->PUPDR |=  (1 << (6*2));
    GPIOB->AFR[0] &= ~(0xF << (6*4));
    GPIOB->AFR[0] |=  (4 << (6*4));

    // PB7 SDA
    GPIOB->MODER &= ~(3 << (7*2));
    GPIOB->MODER |=  (2 << (7*2));
    GPIOB->OTYPER |= (1 << 7);
    GPIOB->OSPEEDR |= (3 << (7*2));
    GPIOB->PUPDR &= ~(3 << (7*2));
    GPIOB->PUPDR |=  (1 << (7*2));
    GPIOB->AFR[0] &= ~(0xF << (7*4));
    GPIOB->AFR[0] |=  (4 << (7*4));

    /* ================= USER BUTTON PC13 ================= */
    GPIOC->MODER &= ~(3 << (13 * 2));   // Input

    GPIOC->PUPDR &= ~(3 << (13 * 2));
    GPIOC->PUPDR |=  (1 << (13 * 2));   // Pull-up
}

uint8_t Button_Pressed(void)
{
    return ((GPIOC->IDR & (1 << 13)) == 0);  // Active LOW
}


void GPIO_Init_Output(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    GPIOx->MODER &= ~(3 << (pin * 2));
    GPIOx->MODER |=  (1 << (pin * 2));

    GPIOx->OTYPER &= ~(1 << pin);
    GPIOx->OSPEEDR |= (3 << (pin * 2));
    GPIOx->PUPDR &= ~(3 << (pin * 2));
}

void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    GPIOx->ODR ^= (1 << pin);
}
