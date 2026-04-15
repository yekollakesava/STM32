#include "main.h"
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/* Handles */
UART_HandleTypeDef huart2;
TaskHandle_t ledTaskHandle;

/* Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void Error_Handler(void);
void led_task(void *params);
void uart_task(void *params);

/* Redirect printf to UART */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    xTaskCreate(uart_task, "UART_TASK", 256, NULL, 1, NULL);
    xTaskCreate(led_task, "LED_TASK", 256, NULL, 2, &ledTaskHandle);

    vTaskStartScheduler();

    while(1);
}

/* UART task */
void uart_task(void *params)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    printf("System Initialized\r\nUART Task Running\r\n");

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/* LED task */
void led_task(void *params)
{
    uint32_t notifyVal;

    while(1)
    {
        xTaskNotifyWait(0, 0, &notifyVal, portMAX_DELAY);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        printf("Button Press Detected! LED toggled\r\n");
    }
}

/* EXTI callback */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t higherPriorityWoken = pdFALSE;
    if(GPIO_Pin == GPIO_PIN_13)
    {
        /* Notify LED task from ISR */
        xTaskNotifyFromISR(ledTaskHandle, 1, eSetValueWithOverwrite, &higherPriorityWoken);
        portYIELD_FROM_ISR(higherPriorityWoken);
    }
}

/* GPIO Init */
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
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Nucleo already has pull-up
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Enable EXTI15_10 IRQ with priority safe for FreeRTOS */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0); // <= configMAX_SYSCALL_INTERRUPT_PRIORITY
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* UART2 Init */
static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&huart2) != HAL_OK)
        Error_Handler();
}

/* System Clock HSI 16 MHz */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    while(1);
}
