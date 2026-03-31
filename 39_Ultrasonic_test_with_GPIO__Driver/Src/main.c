#include "stm32l47xx_gpio_driver.h"
#include <stdio.h>
#include <stdint.h>

#define TRIG_PIN            GPIO_PIN_NO_0
#define ECHO_PIN            GPIO_PIN_NO_1

#define ULTRASONIC_DIVISOR  16U

void UART2_Init(void);
void UART2_SendChar(char c);
void UART2_SendString(char *str);

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

void Ultrasonic_Init(void);
void Ultrasonic_Trigger(void);
uint32_t Ultrasonic_MeasureEcho(void);
uint32_t Ultrasonic_GetDistance(void);
uint32_t Ultrasonic_GetDistance_Avg(uint8_t samples);

/************************************
UART INIT
PA2 -> TX
PA3 -> RX
9600 baud @ 4 MHz
************************************/
void UART2_Init(void)
{
    USART2_PCLK_EN();
    GPIOA_PCLK_EN();

    /* PA2, PA3 alternate mode */
    GPIOA->MODER &= ~(0xFU << 4);
    GPIOA->MODER |=  (0xAU << 4);

    /* AF7 for USART2 */
    GPIOA->AFR[0] &= ~(0xFFU << 8);
    GPIOA->AFR[0] |=  (0x77U << 8);

    /* pull-up */
    GPIOA->PUPDR &= ~(0xFU << 4);
    GPIOA->PUPDR |=  (0x5U << 4);

    /* optional: push-pull, medium speed */
    GPIOA->OTYPER &= ~((1U << 2) | (1U << 3));
    GPIOA->OSPEEDR &= ~(0xFU << 4);
    GPIOA->OSPEEDR |=  (0x5U << 4);

    USART2->BRR = 0x01A0U;   /* 9600 baud for 4 MHz */

    USART2->CR1 = 0;
    USART2->CR1 |= (1U << USART_CR1_TE);
    USART2->CR1 |= (1U << USART_CR1_RE);
    USART2->CR1 |= (1U << USART_CR1_UE);
}

/************************************
UART SEND CHAR
************************************/
void UART2_SendChar(char c)
{
    while(!(USART2->ISR & (1U << USART_ISR_TXE)));
    USART2->TDR = (uint8_t)c;
}

/************************************
UART SEND STRING
************************************/
void UART2_SendString(char *str)
{
    while(*str)
    {
        UART2_SendChar(*str++);
    }

    while(!(USART2->ISR & (1U << USART_ISR_TC)));
}

/************************************
SOFTWARE DELAY
Note: rough delay only, not accurate µs
************************************/
void delay_us(uint32_t us)
{
    while(us--)
    {
        for(volatile uint32_t i = 0; i < 4; i++);
    }
}

void delay_ms(uint32_t ms)
{
    while(ms--)
    {
        delay_us(1000);
    }
}

/************************************
ULTRASONIC GPIO INIT
PA0 -> TRIG output
PA1 -> ECHO input
************************************/
void Ultrasonic_Init(void)
{
    GPIO_Handle_t GpioTrig, GpioEcho;

    GPIO_PeriClockControl(GPIOA, ENABLE);

    /* TRIG output */
    GpioTrig.pGPIOx = GPIOA;
    GpioTrig.GPIO_PinConfig.GPIO_PinNumber = TRIG_PIN;
    GpioTrig.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    GpioTrig.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GpioTrig.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    GpioTrig.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GpioTrig.GPIO_PinConfig.GPIO_PinAltFunMode = 0;
    GPIO_Init(&GpioTrig);

    /* ECHO input */
    GpioEcho.pGPIOx = GPIOA;
    GpioEcho.GPIO_PinConfig.GPIO_PinNumber = ECHO_PIN;
    GpioEcho.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    GpioEcho.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GpioEcho.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    GpioEcho.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GpioEcho.GPIO_PinConfig.GPIO_PinAltFunMode = 0;
    GPIO_Init(&GpioEcho);

    GPIO_WriteToOutputPin(GPIOA, TRIG_PIN, GPIO_PIN_RESET);
}

/************************************
SEND 10us TRIGGER PULSE
************************************/
void Ultrasonic_Trigger(void)
{
    GPIO_WriteToOutputPin(GPIOA, TRIG_PIN, GPIO_PIN_RESET);
    delay_us(3);

    GPIO_WriteToOutputPin(GPIOA, TRIG_PIN, GPIO_PIN_SET);
    delay_us(10);

    GPIO_WriteToOutputPin(GPIOA, TRIG_PIN, GPIO_PIN_RESET);
}

/************************************
MEASURE ECHO HIGH TIME
Returns loop count, not real microseconds
************************************/
uint32_t Ultrasonic_MeasureEcho(void)
{
    uint32_t time_count = 0;
    uint32_t timeout = 30000;

    /* wait for echo to go HIGH */
    while(GPIO_ReadFromInputPin(GPIOA, ECHO_PIN) == 0)
    {
        if(timeout-- == 0)
            return 0;
        delay_us(1);
    }

    timeout = 30000;

    /* count while echo stays HIGH */
    while(GPIO_ReadFromInputPin(GPIOA, ECHO_PIN) == 1)
    {
        if(timeout-- == 0)
            break;

        time_count++;
        delay_us(1);
    }

    return time_count;
}

/************************************
GET SINGLE DISTANCE
************************************/
uint32_t Ultrasonic_GetDistance(void)
{
    uint32_t echo_time;

    Ultrasonic_Trigger();
    echo_time = Ultrasonic_MeasureEcho();

    if(echo_time == 0)
        return 0;

    return (echo_time / ULTRASONIC_DIVISOR);
}

/************************************
AVERAGE MULTIPLE SAMPLES
************************************/
uint32_t Ultrasonic_GetDistance_Avg(uint8_t samples)
{
    uint32_t sum = 0;
    uint32_t dist = 0;
    uint8_t valid = 0;

    for(uint8_t i = 0; i < samples; i++)
    {
        dist = Ultrasonic_GetDistance();

        if(dist > 0)
        {
            sum += dist;
            valid++;
        }

        delay_ms(20);
    }

    if(valid == 0)
        return 0;

    return (sum / valid);
}

/************************************
MAIN
************************************/
int main(void)
{
    char buffer[64];
    uint32_t distance;

    UART2_Init();
    Ultrasonic_Init();

    while(1)
    {
        distance = Ultrasonic_GetDistance_Avg(5);

        sprintf(buffer, "Distance = %lu cm\r\n", distance);
        UART2_SendString(buffer);

        delay_ms(50);
    }
}
