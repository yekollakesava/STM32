#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart2;
extern SemaphoreHandle_t  myCountingSemHandle;

/* ================= PRODUCER TASK ================= */
void ProducerTask_func(void const * argument)
{
  uint8_t count = 0;

  osDelay(200);
  printf("[Producer] Task started\r\n");

  for(;;)
  {
    osDelay(1000);

    if(xSemaphoreGive(myCountingSemHandle) == pdTRUE)
    {
      count++;
      printf("[Producer] Produced item %d  | sem count: %lu\r\n",
              count,
              uxSemaphoreGetCount(myCountingSemHandle));
    }
    else
    {
      printf("[Producer] Semaphore FULL! (max=3)\r\n");
    }
  }
}

/* ================= CONSUMER TASK ================= */
void ConsumerTask_func(void const * argument)
{
  uint8_t consumed = 0;

  osDelay(400);
  printf("[Consumer] Task started\r\n");

  for(;;)
  {
    if(xSemaphoreTake(myCountingSemHandle, portMAX_DELAY) == pdTRUE)
    {
      consumed++;
      printf("[Consumer] Consumed item %d | sem count: %lu\r\n",
              consumed,
              uxSemaphoreGetCount(myCountingSemHandle));

      osDelay(2000);
    }
  }
}

/* ================= IDLE TASK MEMORY ================= */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t  xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t  **ppxIdleTaskStackBuffer,
                                   uint32_t      *pulIdleTaskStackSize)
{
  *ppxIdleTaskTCBBuffer   = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = xIdleStack;
  *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}
