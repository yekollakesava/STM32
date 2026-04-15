#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* UART handle */
UART_HandleTypeDef huart2;

/* Mutex handle */
SemaphoreHandle_t uartMutex = NULL;

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void Task1_Function(void *argument);
void Task2_Function(void *argument);
void Error_Handler(void);

/* printf redirect to UART */
#if defined(__GNUC__)
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
#endif

/* Safe UART print using mutex */
void UART_Print(const char *msg)
{
    if (uartMutex != NULL)
    {
        if (xSemaphoreTake(uartMutex, portMAX_DELAY) == pdTRUE)
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            xSemaphoreGive(uartMutex);
        }
    }
}

/* Task 1 */
void Task1_Function(void *argument)
{
    (void)argument;

    while (1)
    {
        UART_Print("Task 1: Using shared UART resource\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Task 2 */
void Task2_Function(void *argument)
{
    (void)argument;

    while (1)
    {
        UART_Print("Task 2: Using shared UART resource\r\n");
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();

    /* Create mutex */
    uartMutex = xSemaphoreCreateMutex();

    if (uartMutex == NULL)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"Mutex creation FAILED\r\n",
                          strlen("Mutex creation FAILED\r\n"), HAL_MAX_DELAY);
        Error_Handler();
    }

    UART_Print("\r\n=== FreeRTOS Mutex Demo Start ===\r\n");

    /* Create tasks */
    if (xTaskCreate(Task1_Function, "Task1", 256, NULL, 2, NULL) != pdPASS)
    {
        UART_Print("Task1 creation FAILED\r\n");
        Error_Handler();
    }

    if (xTaskCreate(Task2_Function, "Task2", 256, NULL, 2, NULL) != pdPASS)
    {
        UART_Print("Task2 creation FAILED\r\n");
        Error_Handler();
    }

    /* Start scheduler */
    vTaskStartScheduler();

    while (1)
    {
    }
}

/* USART2 init */
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
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* GPIO init */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/* Replace this with CubeMX generated clock config */
void SystemClock_Config(void)
{
}

/* Error handler */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
