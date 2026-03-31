#ifndef INC_STM32L47XX_GPIO_DRIVER_H_
#define INC_STM32L47XX_GPIO_DRIVER_H_

#include "stm32l47xx.h"

/*
 * GPIO pin configuration structure
 */
typedef struct
{
    uint8_t GPIO_PinNumber;
    uint8_t GPIO_PinMode;
    uint8_t GPIO_PinSpeed;
    uint8_t GPIO_PinPuPdControl;
    uint8_t GPIO_PinOPType;
    uint8_t GPIO_PinAltFunMode;
} GPIO_PinConfig_t;

/*
 * GPIO handle structure
 */
typedef struct
{
    GPIO_RegDef_t *pGPIOx;
    GPIO_PinConfig_t GPIO_PinConfig;
} GPIO_Handle_t;

/*
 * GPIO pin numbers
 */
#define GPIO_PIN_NO_0           0
#define GPIO_PIN_NO_1           1
#define GPIO_PIN_NO_2           2
#define GPIO_PIN_NO_3           3
#define GPIO_PIN_NO_4           4
#define GPIO_PIN_NO_5           5
#define GPIO_PIN_NO_6           6
#define GPIO_PIN_NO_7           7
#define GPIO_PIN_NO_8           8
#define GPIO_PIN_NO_9           9
#define GPIO_PIN_NO_10          10
#define GPIO_PIN_NO_11          11
#define GPIO_PIN_NO_12          12
#define GPIO_PIN_NO_13          13
#define GPIO_PIN_NO_14          14
#define GPIO_PIN_NO_15          15

/*
 * GPIO modes
 */
#define GPIO_MODE_IN            0
#define GPIO_MODE_OUT           1
#define GPIO_MODE_ALTFN         2
#define GPIO_MODE_ANALOG        3
#define GPIO_MODE_IT_FT         4
#define GPIO_MODE_IT_RT         5
#define GPIO_MODE_IT_RFT        6

/*
 * GPIO output types
 */
#define GPIO_OP_TYPE_PP         0
#define GPIO_OP_TYPE_OD         1

/*
 * GPIO speed
 */
#define GPIO_SPEED_LOW          0
#define GPIO_SPEED_MEDIUM       1
#define GPIO_SPEED_FAST         2
#define GPIO_SPEED_HIGH         3

/*
 * GPIO pull-up / pull-down
 */
#define GPIO_NO_PUPD            0
#define GPIO_PIN_PU             1
#define GPIO_PIN_PD             2

/*
 * GPIO port code
 */
#define GPIO_BASEADDR_TO_CODE(x)   ( (x == GPIOA) ? 0 : \
                                     (x == GPIOB) ? 1 : \
                                     (x == GPIOC) ? 2 : \
                                     (x == GPIOD) ? 3 : \
                                     (x == GPIOE) ? 4 : \
                                     (x == GPIOF) ? 5 : \
                                     (x == GPIOG) ? 6 : 0 )

/*
 * IRQ numbers
 */
#define IRQ_NO_EXTI0            6
#define IRQ_NO_EXTI1            7
#define IRQ_NO_EXTI2            8
#define IRQ_NO_EXTI3            9
#define IRQ_NO_EXTI4            10
#define IRQ_NO_EXTI9_5          23
#define IRQ_NO_EXTI15_10        40

/*
 * IRQ priorities
 */
#define NVIC_IRQ_PRI0           0
#define NVIC_IRQ_PRI1           1
#define NVIC_IRQ_PRI2           2
#define NVIC_IRQ_PRI3           3
#define NVIC_IRQ_PRI4           4
#define NVIC_IRQ_PRI5           5
#define NVIC_IRQ_PRI6           6
#define NVIC_IRQ_PRI7           7
#define NVIC_IRQ_PRI8           8
#define NVIC_IRQ_PRI9           9
#define NVIC_IRQ_PRI10          10
#define NVIC_IRQ_PRI11          11
#define NVIC_IRQ_PRI12          12
#define NVIC_IRQ_PRI13          13
#define NVIC_IRQ_PRI14          14
#define NVIC_IRQ_PRI15          15

/*
 * APIs
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnOrDi);
void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value);
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value);
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);

void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void GPIO_IRQHandling(uint16_t PinNumber);

#endif
