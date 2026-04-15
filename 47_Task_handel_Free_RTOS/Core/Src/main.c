#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

/* Task Handles */
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

void Task1(void *argument);
void Task2(void *argument);

/* Priority flag */
volatile uint8_t reversed = 0;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* Create Tasks */
  xTaskCreate(Task1, "Task1", 128, NULL, 2, &Task1Handle);
  xTaskCreate(Task2, "Task2", 128, NULL, 3, &Task2Handle);

  /* Start Scheduler */
  vTaskStartScheduler();

  while (1)
  {
  }
}

/* Task1 : Toggle LED every 100ms */
void Task1(void *argument)
{
  while(1)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/* Task2 : Toggle LED every 1 second */
void Task2(void *argument)
{
  while(1)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/* Button Interrupt */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_13)
  {
      if(reversed == 0)
      {
          vTaskPrioritySet(Task1Handle, 3);
          vTaskPrioritySet(Task2Handle, 2);
          reversed = 1;
      }
      else
      {
          vTaskPrioritySet(Task1Handle, 2);
          vTaskPrioritySet(Task2Handle, 3);
          reversed = 0;
      }
  }
}

/* GPIO Configuration */
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* LED PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Button PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_RCC_DeInit();

  /* Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Initializes the CPU, AHB and APB buses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1);
  }

  /* Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    while(1);
  }
}
