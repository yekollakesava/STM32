/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "string.h"
#include "stdio.h"

/* USER CODE BEGIN PD */
#define FLASH_USER_START_ADDR   0x0807F800
#define FLASH_PAGE_NUMBER       255
/* USER CODE END PD */

TIM_HandleTypeDef htim1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);

/* USER CODE BEGIN 0 */

// Microsecond delay — timer must already be running
// Simple us delay — does NOT reset counter globally
void delay_us(uint16_t us) {
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim1);
    while ((__HAL_TIM_GET_COUNTER(&htim1) - start) < us);
}

float get_distance_cm(void) {
    uint32_t t_start, t_stop;
    uint32_t timeout_start;

    // Send 10us TRIG pulse
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

    // Wait for ECHO HIGH — timeout 30ms
    timeout_start = __HAL_TIM_GET_COUNTER(&htim1);
    while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET) {
        if ((__HAL_TIM_GET_COUNTER(&htim1) - timeout_start) > 30000)
            return 0.0f;
    }

    // Record start time
    t_start = __HAL_TIM_GET_COUNTER(&htim1);

    // Wait for ECHO LOW — timeout 30ms
    while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET) {
        if ((__HAL_TIM_GET_COUNTER(&htim1) - t_start) > 30000)
            return 0.0f;
    }

    // Record stop time
    t_stop = __HAL_TIM_GET_COUNTER(&htim1);

    // Calculate pulse width (handles timer overflow too)
    uint32_t pulse_us = t_stop - t_start;

    return (pulse_us * 0.034f) / 2.0f;
}
// Write 64-bit value to internal flash
HAL_StatusTypeDef Flash_Write(uint32_t address, uint64_t data) {
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError;
    char dbg[64];

    __disable_irq();
    HAL_FLASH_Unlock();

    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.Banks     = FLASH_BANK_1;   // 0x0807F800 is in Bank 1
    eraseInit.Page      = FLASH_PAGE_NUMBER;
    eraseInit.NbPages   = 1;

    status = HAL_FLASHEx_Erase(&eraseInit, &pageError);
    if (status != HAL_OK) {
        snprintf(dbg, sizeof(dbg), "Erase failed! 0x%08lX\r\n", pageError);
        HAL_UART_Transmit(&huart2, (uint8_t*)dbg, strlen(dbg), HAL_MAX_DELAY);
        HAL_FLASH_Lock();
        __enable_irq();
        return HAL_ERROR;
    }

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
    if (status != HAL_OK) {
        snprintf(dbg, sizeof(dbg), "Program failed! err=%d\r\n", status);
        HAL_UART_Transmit(&huart2, (uint8_t*)dbg, strlen(dbg), HAL_MAX_DELAY);
    }

    HAL_FLASH_Lock();
    __enable_irq();
    return status;
}

// Read 64-bit value from flash
uint64_t Flash_Read(uint32_t address) {
    return *(__IO uint64_t*)address;
}

/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_TIM1_Init();

    /* USER CODE BEGIN 2 */
    char msg[64];

    // Start timer ONCE here — keep it running always
    HAL_TIM_Base_Start(&htim1);

    // Small startup delay
    HAL_Delay(500);
    HAL_UART_Transmit(&huart2, (uint8_t*)"System Ready!\r\n", 15, HAL_MAX_DELAY);
    /* USER CODE END 2 */

    while (1)
    {
        /* USER CODE BEGIN WHILE */

        // Step 1: Get distance from sensor
        float distance = get_distance_cm();

        // Step 2: Convert float to uint64 for flash storage
        uint64_t flash_data = 0;
        memcpy(&flash_data, &distance, sizeof(float));

        // Step 3: Write to internal flash
        HAL_StatusTypeDef result = Flash_Write(FLASH_USER_START_ADDR, flash_data);

        // Step 4: Read back from flash
        uint64_t read_raw = Flash_Read(FLASH_USER_START_ADDR);
        float read_distance = 0.0f;
        memcpy(&read_distance, &read_raw, sizeof(float));

        // Step 5: Send result over UART
        if (result == HAL_OK) {
            snprintf(msg, sizeof(msg),
                     "Written: %.2f cm | Read back: %.2f cm\r\n",
                     distance, read_distance);
        } else {
            snprintf(msg, sizeof(msg), "Flash write FAILED!\r\n");
        }

        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        HAL_Delay(2000);

        /* USER CODE END WHILE */
    }
}

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
    RCC_OscInitStruct.PLL.PLLM            = 1;
    RCC_OscInitStruct.PLL.PLLN            = 10;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
        Error_Handler();
}

static void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim1.Instance               = TIM1;
    htim1.Init.Prescaler         = 79;   // 80MHz / (79+1) = 1MHz = 1us per tick
    htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim1.Init.Period            = 65535;
    htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
        Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
        Error_Handler();

    sMasterConfig.MasterOutputTrigger  = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
        Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance                    = USART2;
    huart2.Init.BaudRate               = 115200;
    huart2.Init.WordLength             = UART_WORDLENGTH_8B;
    huart2.Init.StopBits               = UART_STOPBITS_1;
    huart2.Init.Parity                 = UART_PARITY_NONE;
    huart2.Init.Mode                   = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
        Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, LD2_Pin | TRIG_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin  = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = LD2_Pin | TRIG_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin  = ECHO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ECHO_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
