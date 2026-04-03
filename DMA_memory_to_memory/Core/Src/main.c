#include "stm32l476xx.h"
#include <stdint.h>

#define DATA_LENGTH 10

uint32_t srcBuffer[DATA_LENGTH] = {11, 22, 33, 44, 55, 66, 77, 88, 99, 111};
uint32_t dstBuffer[DATA_LENGTH] = {0};

static void GPIOA_LED_Init(void);
static void DMA1_Mem2Mem_Init(void);
static void DMA1_Mem2Mem_Start(uint32_t *src, uint32_t *dst, uint16_t len);
static uint8_t Buffer_Verify(uint32_t *src, uint32_t *dst, uint16_t len);
static void delay(volatile uint32_t count);

int main(void)
{
    GPIOA_LED_Init();
    DMA1_Mem2Mem_Init();

    DMA1_Mem2Mem_Start(srcBuffer, dstBuffer, DATA_LENGTH);

    /* Wait until transfer complete */
    while (!(DMA1->ISR & DMA_ISR_TCIF1))
    {
        /* wait */
    }

    /* Clear transfer complete flag */
    DMA1->IFCR |= DMA_IFCR_CTCIF1;

    if (Buffer_Verify(srcBuffer, dstBuffer, DATA_LENGTH))
    {
        /* Success: slow blink */
        while (1)
        {
            GPIOA->ODR ^= (1U << 5);
            delay(500000);
        }
    }
    else
    {
        /* Failure: fast blink */
        while (1)
        {
            GPIOA->ODR ^= (1U << 5);
            delay(100000);
        }
    }
}

static void GPIOA_LED_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* PA5 -> output mode */
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |=  (1U << (5 * 2));

    /* Push-pull */
    GPIOA->OTYPER &= ~(1U << 5);

    /* Medium speed */
    GPIOA->OSPEEDR &= ~(3U << (5 * 2));
    GPIOA->OSPEEDR |=  (1U << (5 * 2));

    /* No pull-up/pull-down */
    GPIOA->PUPDR &= ~(3U << (5 * 2));
}

static void DMA1_Mem2Mem_Init(void)
{
    /* Enable DMA1 clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    /* Disable channel before configuration */
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    /* Clear all flags for Channel 1 */
    DMA1->IFCR |= DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1;

    /*
     * DMA Channel1 configuration:
     * - Memory to memory mode enabled
     * - Source increment enabled
     * - Destination increment enabled
     * - Source size = 32-bit
     * - Destination size = 32-bit
     * - Normal mode
     * - High priority
     *
     * In memory-to-memory mode:
     * - CPAR is used as source address
     * - CMAR is used as destination address
     */
    DMA1_Channel1->CCR =
          DMA_CCR_MEM2MEM      /* Memory-to-memory mode */
        | DMA_CCR_MINC         /* Increment destination address */
        | DMA_CCR_PINC         /* Increment source address */
        | DMA_CCR_MSIZE_1      /* Memory size = 32-bit */
        | DMA_CCR_PSIZE_1      /* Peripheral size = 32-bit */
        | DMA_CCR_PL_1;        /* Priority = High */
}

static void DMA1_Mem2Mem_Start(uint32_t *src, uint32_t *dst, uint16_t len)
{
    /* Disable channel before loading values */
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    /* Number of data items */
    DMA1_Channel1->CNDTR = len;

    /* Source address */
    DMA1_Channel1->CPAR = (uint32_t)src;

    /* Destination address */
    DMA1_Channel1->CMAR = (uint32_t)dst;

    /* Clear flags again */
    DMA1->IFCR |= DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1;

    /* Enable DMA channel */
    DMA1_Channel1->CCR |= DMA_CCR_EN;
}

static uint8_t Buffer_Verify(uint32_t *src, uint32_t *dst, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        if (src[i] != dst[i])
        {
            return 0;
        }
    }
    return 1;
}

static void delay(volatile uint32_t count)
{
    while (count--)
    {
        __asm("nop");
    }
}
