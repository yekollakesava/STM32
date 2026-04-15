#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/* UART handle */
UART_HandleTypeDef huart2;

/* Task Handle */
TaskHandle_t ledTaskHandle = NULL;

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* Task functions */
void LED_Task(void *params);
void Button_Task(void *params);

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

    printf("FreeRTOS Task Delete Demo\r\n");

    /* Create Button Task */
    xTaskCreate(Button_Task,
                "ButtonTask",
                200,
                NULL,
                2,
                NULL);

    /* Start Scheduler */
    vTaskStartScheduler();

    while (1)
    {
    }
}

/* LED Task */
void LED_Task(void *params)
{
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

        printf("LED Task Running\r\n");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* Button Task */
void Button_Task(void *params)
{
    uint8_t button_state;

    while(1)
    {
        button_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

        if(button_state == GPIO_PIN_RESET)
        {
            vTaskDelay(pdMS_TO_TICKS(200));   // debounce

            if(ledTaskHandle == NULL)
            {
                printf("Creating LED Task\r\n");

                xTaskCreate(LED_Task,
                            "LEDTask",
                            200,
                            NULL,
                            1,
                            &ledTaskHandle);
            }
            else
            {
                printf("Deleting LED Task\r\n");

                vTaskDelete(ledTaskHandle);
                ledTaskHandle = NULL;
            }

            while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/* GPIO Initialization */
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
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/* UART2 Initialization */
static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
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

    HAL_UART_Init(&huart2);
}

/* System Clock Config */
void SystemClock_Config(void)
{
}
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
