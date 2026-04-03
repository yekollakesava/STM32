#include "adc.h"

static volatile uint16_t g_adc_value = 0;
static volatile uint8_t  g_adc_done  = 0;

static void ADC1_GPIO_Init(uint8_t channel)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* For this example: channel 5 = PA0 */
    if(channel == 5)
    {
        /* PA0 analog mode */
        GPIOA->MODER &= ~(3U << (0 * 2));
        GPIOA->MODER |=  (3U << (0 * 2));

        /* No pull-up / pull-down */
        GPIOA->PUPDR &= ~(3U << (0 * 2));
    }
}

static void ADC1_EnableClock(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
}

static void ADC1_PowerUp(void)
{
    /* Exit deep-power-down */
    ADC1->CR &= ~ADC_CR_DEEPPWD;

    /* Enable ADC voltage regulator */
    ADC1->CR |= ADC_CR_ADVREGEN;

    /* small delay */
    for(volatile uint32_t i = 0; i < 10000; i++);
}

static void ADC1_DisableIfEnabled(void)
{
    if(ADC1->CR & ADC_CR_ADEN)
    {
        ADC1->CR |= ADC_CR_ADDIS;
        while(ADC1->CR & ADC_CR_ADEN);
    }
}

static void ADC1_Calibrate(void)
{
    ADC1_DisableIfEnabled();

    /* Single-ended calibration */
    ADC1->CR &= ~ADC_CR_ADCALDIF;
    ADC1->CR |= ADC_CR_ADCAL;

    while(ADC1->CR & ADC_CR_ADCAL);
}

static void ADC1_Enable(void)
{
    /* Clear ADRDY */
    ADC1->ISR |= ADC_ISR_ADRDY;

    ADC1->CR |= ADC_CR_ADEN;

    while((ADC1->ISR & ADC_ISR_ADRDY) == 0);
}

void ADC1_Init(ADC_Config_t *pADCConfig)
{
    ADC1_GPIO_Init(pADCConfig->channel);
    ADC1_EnableClock();
    ADC1_PowerUp();
    ADC1_Calibrate();

    ADC1_DisableIfEnabled();

    /* Resolution
       00 = 12-bit
       01 = 10-bit
       10 = 8-bit
       11 = 6-bit
    */
    ADC1->CFGR &= ~ADC_CFGR_RES;
    ADC1->CFGR |= ((uint32_t)(pADCConfig->resolution & 0x03U) << ADC_CFGR_RES_Pos);

    /* Alignment right */
    ADC1->CFGR &= ~ADC_CFGR_ALIGN;

    /* Continuous mode */
    if(pADCConfig->continuous_mode)
        ADC1->CFGR |= ADC_CFGR_CONT;
    else
        ADC1->CFGR &= ~ADC_CFGR_CONT;

    /* Sequence length = 1 conversion */
    ADC1->SQR1 &= ~ADC_SQR1_L;

    /* Select channel as SQ1 */
    ADC1->SQR1 &= ~ADC_SQR1_SQ1;
    ADC1->SQR1 |= ((uint32_t)pADCConfig->channel << ADC_SQR1_SQ1_Pos);

    /* Sample time */
    if(pADCConfig->channel <= 9)
    {
        uint32_t pos = pADCConfig->channel * 3U;
        ADC1->SMPR1 &= ~(7U << pos);
        ADC1->SMPR1 |= ((uint32_t)(pADCConfig->sample_time & 0x07U) << pos);
    }
    else
    {
        uint32_t pos = (pADCConfig->channel - 10U) * 3U;
        ADC1->SMPR2 &= ~(7U << pos);
        ADC1->SMPR2 |= ((uint32_t)(pADCConfig->sample_time & 0x07U) << pos);
    }

    /* Enable End Of Conversion interrupt */
    ADC1->IER |= ADC_IER_EOCIE;

    /* Enable ADC IRQ in NVIC */
    NVIC_EnableIRQ(ADC1_2_IRQn);

    ADC1_Enable();
}

void ADC1_StartConversion_IT(void)
{
    g_adc_done = 0;

    /* Clear flags */
    ADC1->ISR |= ADC_ISR_EOC;
    ADC1->ISR |= ADC_ISR_EOS;

    /* Start conversion */
    ADC1->CR |= ADC_CR_ADSTART;
}

uint16_t ADC1_GetValue(void)
{
    return g_adc_value;
}

uint8_t ADC1_ConversionComplete(void)
{
    return g_adc_done;
}

void ADC1_ClearConversionFlag(void)
{
    g_adc_done = 0;
}

void ADC1_2_IRQHandler(void)
{
    if(ADC1->ISR & ADC_ISR_EOC)
    {
        g_adc_value = (uint16_t)ADC1->DR;   /* Reading DR clears EOC */
        g_adc_done = 1;
    }
}
