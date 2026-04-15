#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

/* UART handle */
UART_HandleTypeDef huart2;

/* Queue Handle */
QueueHandle_t myQueue;

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* Task Functions */
void Sender_Task(void *params);
void Receiver_Task(void *params);

/* Redirect printf to UART */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2,(uint8_t*)ptr,len,HAL_MAX_DELAY);
    return len;
}

/* Sender Task */
void Sender_Task(void *params)
{
    int value = 10;

    while(1)
    {
        xQueueSend(myQueue,&value,portMAX_DELAY);
        //printf("Data Sent: %d\r\n",value);
        //printf("Data Sent: %d\r\n", value);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Receiver Task */
void Receiver_Task(void *params)
{
    int received;

    while(1)
    {
        if(xQueueReceive(myQueue,&received,portMAX_DELAY)==pdPASS)
        {
            printf("Received: %d\r\n",received);
        }
    }
}

/* Main Function */
int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();

    /* Create Queue */
    myQueue = xQueueCreate(5,sizeof(int));

    /* Create Tasks */
    xTaskCreate(Sender_Task,"Sender",200,NULL,1,NULL);
    xTaskCreate(Receiver_Task,"Receiver",200,NULL,1,NULL);

    /* Start Scheduler */
    vTaskStartScheduler();

    while(1)
    {
    }
}

/* UART2 Initialization */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;

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
  while(1)
  {
    /* Stay here if error occurs */
  }
}
