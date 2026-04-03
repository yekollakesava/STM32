#include "main.h"
#include "i2c_driver.h"
#include <string.h>
#include <stdio.h>

/* Peripheral handles */
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

/* Sensor definitions */
#define TEMP_SENSOR_ADDR   (0x77 << 1)
#define REG_CHIP_ID        0xD0
#define REG_CTRL_MEAS      0xF4
#define REG_TEMP_MSB       0xFA
#define REG_CALIB_START    0x88

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    int32_t  t_fine;
} BMP280_CalibData;

BMP280_CalibData bmpCal;

void UART_Print(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

HAL_StatusTypeDef I2C_Read_Buffer(uint16_t devAddr, uint8_t regAddr, uint8_t *buf, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, devAddr, regAddr, I2C_MEMADD_SIZE_8BIT, buf, len, HAL_MAX_DELAY);
}

uint16_t u16_le(uint8_t lsb, uint8_t msb)
{
    return (uint16_t)(lsb | (msb << 8));
}

int16_t s16_le(uint8_t lsb, uint8_t msb)
{
    return (int16_t)(lsb | (msb << 8));
}

HAL_StatusTypeDef BMP280_ReadChipID(uint8_t *id)
{
    return I2C_Read_Byte(TEMP_SENSOR_ADDR, REG_CHIP_ID, id);
}

HAL_StatusTypeDef BMP280_ReadCalibration(void)
{
    uint8_t calib[6];

    if (I2C_Read_Buffer(TEMP_SENSOR_ADDR, REG_CALIB_START, calib, 6) != HAL_OK)
        return HAL_ERROR;

    bmpCal.dig_T1 = u16_le(calib[0], calib[1]);
    bmpCal.dig_T2 = s16_le(calib[2], calib[3]);
    bmpCal.dig_T3 = s16_le(calib[4], calib[5]);

    return HAL_OK;
}

HAL_StatusTypeDef BMP280_Config(void)
{
    return I2C_Write_Byte(TEMP_SENSOR_ADDR, REG_CTRL_MEAS, 0x27);
}

HAL_StatusTypeDef BMP280_ReadRawTemp(int32_t *rawTemp)
{
    uint8_t data[3];

    if (I2C_Read_Buffer(TEMP_SENSOR_ADDR, REG_TEMP_MSB, data, 3) != HAL_OK)
        return HAL_ERROR;

    *rawTemp = ((int32_t)data[0] << 12) |
               ((int32_t)data[1] << 4)  |
               ((int32_t)data[2] >> 4);

    return HAL_OK;
}

int32_t BMP280_CompensateTemp_x100(int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)bmpCal.dig_T1 << 1))) *
            ((int32_t)bmpCal.dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)bmpCal.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)bmpCal.dig_T1))) >> 12) *
            ((int32_t)bmpCal.dig_T3)) >> 14;

    bmpCal.t_fine = var1 + var2;
    T = (bmpCal.t_fine * 5 + 128) >> 8;

    return T;
}

int main(void)
{
    char msg[100];
    uint8_t chip_id;
    int32_t rawTemp;
    int32_t temp_x100;
    int32_t temp_int;
    int32_t temp_frac;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    UART_Print("Temperature sensor test start\r\n");

    if (BMP280_ReadChipID(&chip_id) != HAL_OK)
    {
        UART_Print("Chip ID read failed\r\n");
        while (1);
    }

    sprintf(msg, "Chip ID = 0x%02X\r\n", chip_id);
    UART_Print(msg);

    if ((chip_id != 0x58) && (chip_id != 0x60))
    {
        UART_Print("Device at 0x77 is not BMP280/BME280\r\n");
        while (1);
    }

    if (BMP280_ReadCalibration() != HAL_OK)
    {
        UART_Print("Calibration read failed\r\n");
        while (1);
    }

    if (BMP280_Config() != HAL_OK)
    {
        UART_Print("Sensor config failed\r\n");
        while (1);
    }

    UART_Print("Temperature reading started\r\n");

    while (1)
    {
        if (BMP280_ReadRawTemp(&rawTemp) == HAL_OK)
        {
            temp_x100 = BMP280_CompensateTemp_x100(rawTemp);

            temp_int  = temp_x100 / 100;
            temp_frac = temp_x100 % 100;
            if (temp_frac < 0)
                temp_frac = -temp_frac;

            sprintf(msg, "Temperature = %ld.%02ld C\r\n", temp_int, temp_frac);
            UART_Print(msg);
        }
        else
        {
            UART_Print("Temperature read failed\r\n");
        }

        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(1000);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_RCC_DeInit();

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        Error_Handler();
    }
}

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

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
