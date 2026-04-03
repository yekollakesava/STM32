#include "stm32l476xx.h"
#include "adc.h"
#include <stdint.h>

volatile uint16_t adc_result = 0;

static void LED_Init(void)
{
    /* PA5 = onboard LED */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |=  (1U << (5 * 2));
}

static void LED_Toggle(void)
{
    GPIOA->ODR ^= (1U << 5);
}

static void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 4000; i++);
}

int main(void)
{
    ADC_Config_t adc_cfg;

    LED_Init();

    /* ADC1 channel 5 = PA0 */
    adc_cfg.channel = 5;
    adc_cfg.resolution = 0;       /* 12-bit */
    adc_cfg.continuous_mode = 0;  /* single conversion */
    adc_cfg.sample_time = 6;      /* 247.5 cycles */

    ADC1_Init(&adc_cfg);

    while(1)
    {
        ADC1_StartConversion_IT();

        while(!ADC1_ConversionComplete())
        {
            /* wait until interrupt sets flag */
        }

        adc_result = ADC1_GetValue();
        ADC1_ClearConversionFlag();

        /* ADC conversion completed */
        LED_Toggle();

        delay_ms(500);
    }
}
