#ifndef ADC_H
#define ADC_H

#include "stm32l476xx.h"
#include <stdint.h>

typedef struct
{
    uint8_t channel;
    uint8_t resolution;
    uint8_t continuous_mode;
    uint8_t sample_time;
} ADC_Config_t;

void ADC1_Init(ADC_Config_t *pADCConfig);
void ADC1_StartConversion_IT(void);
uint16_t ADC1_GetValue(void);
uint8_t ADC1_ConversionComplete(void);
void ADC1_ClearConversionFlag(void);

#endif
