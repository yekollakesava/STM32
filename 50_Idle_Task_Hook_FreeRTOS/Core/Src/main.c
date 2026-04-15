/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void Task1(void *argument);

/* USER CODE BEGIN PV */
volatile uint32_t idleCounter = 0;
/* USER CODE END PV */

int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  /* Create Task */
  xTaskCreate(Task1, "Task1", 128, NULL, 2, NULL);

  /* Start scheduler */
  vTaskStartScheduler();

  /* Infinite loop */
  while (1)
  {
  }
}

/* Task to simulate CPU load */
void Task1(void *argument)
{
  while (1)
  {
    // Simulate some processing
    for(volatile int i = 0; i < 1000000; i++);
  }
}

/* Idle Hook Function */
void vApplicationIdleHook(void)
{
  static uint32_t count = 0;

  idleCounter++;   // For CPU load observation

  count++;
  if(count > 500000)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Toggle LED
    count = 0;
  }

  // Optional low power mode
  // __WFI();
}

/* GPIO Initialization */
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* System Clock Config (basic template) */
void SystemClock_Config(void)
{
  // Use default CubeMX generated clock config here
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
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
