#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

void button_task(void *argument);
void led_task(void *argument);

volatile uint8_t button_flag = 0;

int main(void)
{

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  xTaskCreate(button_task, "BUTTON_TASK", 128, NULL, 2, NULL);
  xTaskCreate(led_task, "LED_TASK", 128, NULL, 2, NULL);

  vTaskStartScheduler();

  while (1)
  {

  }
}

void button_task(void *argument)
{
  while(1)
  {
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
      {
          button_flag = 1;
      }
      else
      {
          button_flag = 0;
      }

      vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void led_task(void *argument)
{
  while(1)
  {
      if(button_flag == 1)
      {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
      }
      else
      {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
      }

      vTaskDelay(pdMS_TO_TICKS(50));
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SystemClock_Config(void)
{
}
