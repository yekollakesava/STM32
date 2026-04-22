/*
 * main.c – SMART SENTRY FINAL VERSION
 * STM32L476RG | FreeRTOS | HAL
 *
 * IMPORTANT: In FreeRTOSConfig.h set:
 *   #define configTOTAL_HEAP_SIZE  ((size_t)20480)
 *
 * RealTerm settings: 9600 baud, COM = ST-Link Virtual COM Port
 * ESP32 Arduino settings: 115200 baud (USART3)
 */

#include "main.h"
#include "app.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* ── Peripheral handles ──────────────────────────────────────── */
ADC_HandleTypeDef  hadc1;
SPI_HandleTypeDef  hspi2;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
I2C_HandleTypeDef  hi2c1;

/* ── RTOS handles ────────────────────────────────────────────── */
QueueHandle_t      xMotionQueue;
QueueHandle_t      xEnvQueue;
QueueHandle_t      xAlertQueue;
QueueHandle_t      xWifiQueue;
QueueHandle_t      xBtQueue;
QueueHandle_t      xLogQueue;
SemaphoreHandle_t  xUartMutex;
SemaphoreHandle_t  xSpiMutex;
SemaphoreHandle_t  xAdcMutex;
EventGroupHandle_t xSystemEvents;

/* ── Private prototypes ──────────────────────────────────────── */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI2_Init(void);
static void MX_I2C1_Init(void);

/* ══════════════════════════════════════════════════════════════
 *  UART PRINT (pre-RTOS, direct transmit)
 * ══════════════════════════════════════════════════════════════ */
void UART_Print(const char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, (uint16_t)strlen(str), 500);
}

void UART_PrintLine(const char *str)
{
    UART_Print(str);
    HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 100);
}

/* ══════════════════════════════════════════════════════════════
 *  APP_Log – thread-safe UART print + flash log queue
 * ══════════════════════════════════════════════════════════════ */
void APP_Log(const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* UART print only – no flash logging of debug messages */
    if (xUartMutex != NULL &&
        xSemaphoreTake(xUartMutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)buf,
                          (uint16_t)strlen(buf), 300);
        HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 50);
        xSemaphoreGive(xUartMutex);
    }
    else
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)buf,
                          (uint16_t)strlen(buf), 300);
        HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 50);
    }
    /* NOTE: Flash logging removed from APP_Log.
     * Only critical events logged to flash via vLoggerTask directly. */
}


uint32_t APP_GetTick(void)
{
    return (uint32_t)xTaskGetTickCount();
}

/* ══════════════════════════════════════════════════════════════
 *  SELF-TESTS (run before scheduler, direct UART)
 * ══════════════════════════════════════════════════════════════ */
static void SelfTest_GPIO(void)
{
    char buf[80];
    GPIO_PinState ir = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    GPIO_PinState mw = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
    snprintf(buf, sizeof(buf),
             "[GPIO] IR(PA0)=%d  Microwave(PA1)=%d  (0=idle OK)", ir, mw);
    UART_PrintLine(buf);
    UART_PrintLine((ir == 0 && mw == 0)
        ? "[OK] Sensors idle"
        : "[WARN] Sensor HIGH at boot – check wiring");
}

static void SelfTest_I2C(void)
{
    UART_PrintLine("[I2C] Scanning bus...");
    uint8_t found = 0;
    char buf[48];
    for (uint8_t addr = 0x08; addr < 0x78; addr++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1,
            (uint16_t)(addr << 1), 1, 10) == HAL_OK)
        {
            snprintf(buf, sizeof(buf), "[I2C] Device at 0x%02X", addr);
            UART_PrintLine(buf);
            found++;
        }
    }
    if (found == 0)
        UART_PrintLine("[I2C] NO devices found! Check PB8=SCL PB9=SDA + pullups");
    else
    {
        snprintf(buf, sizeof(buf), "[I2C] Scan done: %u device(s)", found);
        UART_PrintLine(buf);
    }

    /* Specific checks */
    UART_PrintLine(
        HAL_I2C_IsDeviceReady(&hi2c1, (0x77<<1), 3, 100) == HAL_OK
        ? "[OK] BME280 at 0x77"
        : "[FAIL] BME280 NOT found at 0x77");
    UART_PrintLine(
        HAL_I2C_IsDeviceReady(&hi2c1, (0x47<<1), 3, 100) == HAL_OK
        ? "[OK] OPT3001 at 0x47"
        : "[FAIL] OPT3001 NOT found at 0x47");
}

static void SelfTest_SPI(void)
{
    uint8_t cmd = 0x9F, id[3] = {0};
    char buf[64];
    FLASH_CS_LOW();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, 100);
    HAL_SPI_Receive(&hspi2, id, 3, 100);
    FLASH_CS_HIGH();
    snprintf(buf, sizeof(buf),
             "[SPI] W25Q JEDEC: %02X %02X %02X", id[0], id[1], id[2]);
    UART_PrintLine(buf);
    UART_PrintLine(id[0] == 0xEF
        ? "[OK] W25Q Flash (Winbond) detected"
        : (id[0] == 0xFF || id[0] == 0x00)
            ? "[FAIL] Flash not responding – check PB12-15"
            : "[WARN] Unknown flash manufacturer");
}

/* ══════════════════════════════════════════════════════════════
 *  RTOS OBJECT CREATION WITH RESULT PRINT
 * ══════════════════════════════════════════════════════════════ */
static void CreateRTOS(void)
{
    UART_PrintLine("[RTOS] Creating queues...");

    /* Queues – sizes chosen for memory balance */
    xMotionQueue = xQueueCreate(4,  sizeof(MotionEvent_t));
    xEnvQueue    = xQueueCreate(1,  sizeof(EnvData_t));      /* overwrite: MUST be 1 */
    xAlertQueue  = xQueueCreate(8,  sizeof(AlertEvent_t));
    xWifiQueue   = xQueueCreate(8,  sizeof(WirelessMsg_t));
    xBtQueue     = xQueueCreate(8,  sizeof(WirelessMsg_t));
    xLogQueue    = xQueueCreate(10, sizeof(LogEntry_t));     /* reduced: 10 not 16 */

    UART_PrintLine(xMotionQueue ? "[OK] xMotionQueue" : "[FAIL] xMotionQueue");
    UART_PrintLine(xEnvQueue    ? "[OK] xEnvQueue"    : "[FAIL] xEnvQueue");
    UART_PrintLine(xAlertQueue  ? "[OK] xAlertQueue"  : "[FAIL] xAlertQueue");
    UART_PrintLine(xWifiQueue   ? "[OK] xWifiQueue"   : "[FAIL] xWifiQueue");
    UART_PrintLine(xBtQueue     ? "[OK] xBtQueue"     : "[FAIL] xBtQueue");
    UART_PrintLine(xLogQueue    ? "[OK] xLogQueue"    : "[FAIL] xLogQueue");

    UART_PrintLine("[RTOS] Creating semaphores...");
    xUartMutex    = xSemaphoreCreateMutex();
    xSpiMutex     = xSemaphoreCreateMutex();
    xAdcMutex     = xSemaphoreCreateMutex();
    xSystemEvents = xEventGroupCreate();

    UART_PrintLine(xUartMutex    ? "[OK] xUartMutex"    : "[FAIL] xUartMutex");
    UART_PrintLine(xSpiMutex     ? "[OK] xSpiMutex"     : "[FAIL] xSpiMutex");
    UART_PrintLine(xAdcMutex     ? "[OK] xAdcMutex"     : "[FAIL] xAdcMutex");
    UART_PrintLine(xSystemEvents ? "[OK] xSystemEvents" : "[FAIL] xSystemEvents");

    /* Assert critical objects */
    configASSERT(xMotionQueue  != NULL);
    configASSERT(xEnvQueue     != NULL);
    configASSERT(xAlertQueue   != NULL);
    configASSERT(xWifiQueue    != NULL);
    configASSERT(xBtQueue      != NULL);
    configASSERT(xLogQueue     != NULL);
    configASSERT(xUartMutex    != NULL);
    configASSERT(xSpiMutex     != NULL);
    configASSERT(xSystemEvents != NULL);
}

static void CreateTasks(void)
{
    BaseType_t r;
    UART_PrintLine("[RTOS] Creating tasks...");

    r = xTaskCreate(vMotionTask,   "Motion",   STACK_MOTION,
                    NULL, PRIORITY_MOTION,   NULL);
    UART_PrintLine(r==pdPASS ? "[OK] Motion Task"   : "[FAIL] Motion Task");

    r = xTaskCreate(vDecisionTask, "Decision", STACK_DECISION,
                    NULL, PRIORITY_DECISION, NULL);
    UART_PrintLine(r==pdPASS ? "[OK] Decision Task" : "[FAIL] Decision Task");

    r = xTaskCreate(vAlertTask,    "Alert",    STACK_ALERT,
                    NULL, PRIORITY_ALERT,    NULL);
    UART_PrintLine(r==pdPASS ? "[OK] Alert Task"    : "[FAIL] Alert Task");

    r = xTaskCreate(vESP32Task,    "ESP32",    STACK_ESP32,
                    NULL, PRIORITY_ESP32,    NULL);
    UART_PrintLine(r==pdPASS ? "[OK] ESP32 Task"    : "[FAIL] ESP32 Task");

    r = xTaskCreate(vEnvTask,      "Env",      STACK_ENV,
                    NULL, PRIORITY_ENV,      NULL);
    UART_PrintLine(r==pdPASS ? "[OK] Env Task"      : "[FAIL] Env Task");

    r = xTaskCreate(vLoggerTask,   "Logger",   STACK_LOGGER,
                    NULL, PRIORITY_LOGGER,   NULL);
    UART_PrintLine(r==pdPASS ? "[OK] Logger Task"   : "[FAIL] Logger Task");
}

/* ══════════════════════════════════════════════════════════════
 *  ISR HANDLERS (EXTI for motion sensors)
 * ══════════════════════════════════════════════════════════════
 * NOTE: Place these in stm32l4xx_it.c if they conflict.
 *       Keep only ONE definition per IRQ across all files.
 * ══════════════════════════════════════════════════════════════ */
void EXTI0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    if (xSystemEvents != NULL)
        xEventGroupSetBitsFromISR(xSystemEvents,
                                  EVT_IR_TRIGGERED,
                                  &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void EXTI1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    if (xSystemEvents != NULL)
        xEventGroupSetBitsFromISR(xSystemEvents,
                                  EVT_MICROWAVE_TRIGGERED,
                                  &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ══════════════════════════════════════════════════════════════
 *  main()
 * ══════════════════════════════════════════════════════════════ */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();   /* Open first – needed for all prints */
    MX_USART3_UART_Init();
    MX_USART1_UART_Init();
    MX_ADC1_Init();
    MX_SPI2_Init();
    MX_I2C1_Init();

    HAL_Delay(200);

    /* ── Boot banner ─────────────────────────────────────────── */
    UART_PrintLine("=========================================");
    UART_PrintLine("  SMART SENTRY  –  STM32L476RG          ");
    UART_PrintLine("  FreeRTOS | BME280 | OPT3001 | ESP32   ");
    UART_PrintLine("=========================================");

    /* ── Hardware self-tests ─────────────────────────────────── */
    SelfTest_GPIO();
    SelfTest_I2C();
    SelfTest_SPI();

    /* ── RTOS object + task creation ─────────────────────────── */
    CreateRTOS();
    CreateTasks();

    /* ── Heap remaining ──────────────────────────────────────── */
    {
        char buf[64];
        snprintf(buf, sizeof(buf),
                 "[HEAP] Free after init: %u bytes",
                 (unsigned)xPortGetFreeHeapSize());
        UART_PrintLine(buf);
    }

    UART_PrintLine("[MAIN] Starting FreeRTOS scheduler...");
    UART_PrintLine("=========================================");

    vTaskStartScheduler();

    UART_PrintLine("[ERROR] Scheduler returned! Heap too small?");
    while (1) {}
}

/* ══════════════════════════════════════════════════════════════
 *  PERIPHERAL INIT FUNCTIONS
 * ══════════════════════════════════════════════════════════════ */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
        Error_Handler();

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM           = 1;
    RCC_OscInitStruct.PLL.PLLN           = 10;
    RCC_OscInitStruct.PLL.PLLP           = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ           = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR           = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
        Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* PA5 – LD2 LED output */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA0 IR EXTI0, PA1 Microwave EXTI1 */
    GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB12 – SPI2 CS (idle HIGH) */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    GPIO_InitStruct.Pin   = GPIO_PIN_12;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PC13 – User Button */
    GPIO_InitStruct.Pin  = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* EXTI priorities */
    HAL_NVIC_SetPriority(EXTI0_IRQn,     5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_SetPriority(EXTI1_IRQn,     5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void MX_ADC1_Init(void)
{
    ADC_MultiModeTypeDef   multimode = {0};
    ADC_ChannelConfTypeDef sConfig   = {0};

    hadc1.Instance                   = ADC1;
    hadc1.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait      = DISABLE;
    hadc1.Init.ContinuousConvMode    = DISABLE;
    hadc1.Init.NbrOfConversion       = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun               = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode      = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
        Error_Handler();

    sConfig.Channel      = ADC_CHANNEL_9;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset       = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}

static void MX_SPI2_Init(void)
{
    hspi2.Instance               = SPI2;
    hspi2.Init.Mode              = SPI_MODE_MASTER;
    hspi2.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi2.Init.NSS               = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi2.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial     = 7;
    hspi2.Init.CRCLength         = SPI_CRC_LENGTH_DATASIZE;
    hspi2.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
    if (HAL_SPI_Init(&hspi2) != HAL_OK) Error_Handler();
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance             = I2C1;
    hi2c1.Init.Timing          = 0x10909CEC; /* 100kHz @ 80MHz */
    hi2c1.Init.OwnAddress1     = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2     = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        Error_Handler();
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
        Error_Handler();
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 9600;   /* RealTerm debug */
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void MX_USART3_UART_Init(void)
{
    huart3.Instance          = USART3;
    huart3.Init.BaudRate     = 115200; /* ESP32 bridge */
    huart3.Init.WordLength   = UART_WORDLENGTH_8B;
    huart3.Init.StopBits     = UART_STOPBITS_1;
    huart3.Init.Parity       = UART_PARITY_NONE;
    huart3.Init.Mode         = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart3) != HAL_OK) Error_Handler();
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    HAL_UART_Transmit(&huart2,(uint8_t*)"\r\nSTACK OVERFLOW: ",19,200);
    HAL_UART_Transmit(&huart2,(uint8_t*)pcTaskName,(uint16_t)strlen(pcTaskName),200);
    HAL_UART_Transmit(&huart2,(uint8_t*)"\r\n",2,50);
    __disable_irq();
    while(1){ HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5); HAL_Delay(100); }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(200);
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file; (void)line;
}
#endif
