#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

/* UART handle */
UART_HandleTypeDef huart2;

/* Binary Semaphore Handle */
SemaphoreHandle_t binSem;

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* Task Prototypes */
void Task1(void *params);
void Task2(void *params);

/* Redirect printf to UART */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2,(uint8_t*)ptr,len,HAL_MAX_DELAY);
    return len;
}

/* Task1 : Waits for semaphore */
void Task1(void *params)
{
    while(1)
    {
        if(xSemaphoreTake(binSem, portMAX_DELAY) == pdTRUE)
        {
            printf("Task1: Semaphore received\r\n");
        }
    }
}

/* Task2 : Gives semaphore */
void Task2(void *params)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));

        printf("Task2: Giving semaphore\r\n");

        xSemaphoreGive(binSem);
    }
}

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART2_UART_Init();

  printf("Binary Semaphore Demo Start\r\n");

  /* Create Binary Semaphore */
  binSem = xSemaphoreCreateBinary();

  if(binSem == NULL)
  {
      printf("Semaphore creation failed\r\n");
  }

  /* Create Tasks */
  xTaskCreate(Task1, "Task1", 200, NULL, 1, NULL);
  xTaskCreate(Task2, "Task2", 200, NULL, 1, NULL);

  /* Start Scheduler */
  vTaskStartScheduler();

  while (1)
  {
  }
}

/* USART2 Initialization */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  HAL_UART_Init(&huart2);
}

/* GPIO Initialization */
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
}

/* System Clock Configuration */
void SystemClock_Config(void)
{
}
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
