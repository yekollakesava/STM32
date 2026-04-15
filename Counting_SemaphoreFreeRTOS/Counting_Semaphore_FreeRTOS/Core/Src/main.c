#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
UART_HandleTypeDef huart2;

osThreadId defaultTaskHandle;
osThreadId ProducerTaskHandle;
osThreadId ConsumerTaskHandle;

/* Counting semaphore handle — native FreeRTOS type */
SemaphoreHandle_t myCountingSemHandle;

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void const * argument);
void ProducerTask_func(void const * argument);
void ConsumerTask_func(void const * argument);

/* UART printf redirect */
#ifdef __GNUC__
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
#endif

/* ================= MAIN ================= */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  setvbuf(stdout, NULL, _IONBF, 0);

  printf("System Started\r\n");

  /* Create counting semaphore: max=3, initial=0 */
  myCountingSemHandle = xSemaphoreCreateCounting(3, 0);

  /* Create tasks */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  osThreadDef(ProducerTask, ProducerTask_func, osPriorityNormal, 0, 256);
  ProducerTaskHandle = osThreadCreate(osThread(ProducerTask), NULL);

  osThreadDef(ConsumerTask, ConsumerTask_func, osPriorityNormal, 0, 256);
  ConsumerTaskHandle = osThreadCreate(osThread(ConsumerTask), NULL);

  osKernelStart();

  while (1) {}
}

/* ================= DEFAULT TASK ================= */
void StartDefaultTask(void const * argument)
{
  for(;;)
  {
    osDelay(1000);
  }
}

/* ================= CLOCK CONFIG ================= */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = 16;
  RCC_OscInitStruct.PLL.PLLN            = 336;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ            = 4;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    Error_Handler();
}

/* ================= GPIO INIT ================= */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* ================= UART INIT ================= */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance          = USART2;
  huart2.Init.BaudRate     = 115200;
  huart2.Init.WordLength   = UART_WORDLENGTH_8B;
  huart2.Init.StopBits     = UART_STOPBITS_1;
  huart2.Init.Parity       = UART_PARITY_NONE;
  huart2.Init.Mode         = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK)
    Error_Handler();
}

/* ================= ERROR HANDLER ================= */
void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
