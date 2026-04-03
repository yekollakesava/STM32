#include "main.h"
#include "stm32l4xx_it.h"

extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;

void DMA1_Channel4_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_spi2_rx);
}

void DMA1_Channel5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_spi2_tx);
}
