#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart2;
osThreadId producerTaskHandle;
osThreadId consumerTaskHandle;
osMessageQId commandQueueHandle;

typedef enum
{
    CMD_LED_ON = 1,
    CMD_LED_OFF,
    CMD_LED_BLINK
} CommandType;

typedef struct
{
    uint8_t command;
    uint16_t blink_delay;
} QueueMessage;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

void StartProducerTask(void *argument);
void StartConsumerTask(void *argument);

void UART_Print(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    osKernelInitialize();

    osMessageQDef(commandQueue, 10, sizeof(QueueMessage));
    commandQueueHandle = osMessageCreate(osMessageQ(commandQueue), NULL);

    osThreadDef(producerTask, StartProducerTask, osPriorityNormal, 0, 256);
    producerTaskHandle = osThreadCreate(osThread(producerTask), NULL);

    osThreadDef(consumerTask, StartConsumerTask, osPriorityAboveNormal, 0, 256);
    consumerTaskHandle = osThreadCreate(osThread(consumerTask), NULL);

    osKernelStart();

    while (1) {}
}

void StartProducerTask(void *argument)
{
    QueueMessage msg;

    while (1)
    {
        msg.command = CMD_LED_ON;
        msg.blink_delay = 0;
        osMessagePut(commandQueueHandle, (uint32_t)&msg, 0, 0);
        UART_Print("Queue: LED ON command sent\r\n");
        osDelay(2000);

        msg.command = CMD_LED_OFF;
        msg.blink_delay = 0;
        osMessagePut(commandQueueHandle, (uint32_t)&msg, 0, 0);
        UART_Print("Queue: LED OFF command sent\r\n");
        osDelay(2000);

        msg.command = CMD_LED_BLINK;
        msg.blink_delay = 300;
        osMessagePut(commandQueueHandle, (uint32_t)&msg, 0, 0);
        UART_Print("Queue: LED BLINK command sent\r\n");
        osDelay(3000);
    }
}

void StartConsumerTask(void *argument)
{
    QueueMessage rxMsg;
    osEvent evt;
    char buffer[64];

    while (1)
    {
        evt = osMessageGet(commandQueueHandle, osWaitForever);
        if (evt.status == osEventMessage)
        {
            memcpy(&rxMsg, evt.value.p, sizeof(QueueMessage));

            switch (rxMsg.command)
            {
                case CMD_LED_ON:
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                    UART_Print("Consumer: LED turned ON\r\n");
                    break;

                case CMD_LED_OFF:
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                    UART_Print("Consumer: LED turned OFF\r\n");
                    break;

                case CMD_LED_BLINK:
                    sprintf(buffer, "Consumer: LED blinking, delay=%d ms\r\n", rxMsg.blink_delay);
                    UART_Print(buffer);

                    for (int i = 0; i < 10; i++)
                    {
                        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
                        osDelay(rxMsg.blink_delay);
                    }
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                    break;

                default:
                    UART_Print("Consumer: Unknown command\r\n");
                    break;
            }
        }
    }
}

// Keep your existing MX_USART2_UART_Init and MX_GPIO_Init functions unchanged
// Add these if missing:

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
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
