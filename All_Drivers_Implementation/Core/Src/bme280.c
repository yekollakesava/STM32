#include "bme280.h"
#include "i2c.h"

#define BME280_ADDR 0x77

#define REG_TEMP_MSB  0xFA
#define REG_CTRL_MEAS 0xF4
#define REG_CALIB     0x88

static uint16_t dig_T1;
static int16_t  dig_T2;
static int16_t  dig_T3;
static int32_t  t_fine;

void BME280_Init(void)
{
	uint8_t id = I2C_ReadReg(BME280_ADDR, 0xD0);
	printf("Chip ID: %X\r\n", id);
    // Soft reset
    I2C_WriteReg(BME280_ADDR, 0xE0, 0xB6);

    for(volatile int i=0;i<100000;i++);

    // Humidity oversampling x1
    I2C_WriteReg(BME280_ADDR, 0xF2, 0x01);

    // Temp x1, pressure x1, normal mode
    I2C_WriteReg(BME280_ADDR, 0xF4, 0x27);

    // Standby config
    I2C_WriteReg(BME280_ADDR, 0xF5, 0xA0);

    // 🔥 LOAD CALIBRATION VALUES
    BME280_ReadCalibration();
}

void BME280_ReadCalibration(void)
{
    uint8_t lsb, msb;

    lsb = I2C_ReadReg(BME280_ADDR, 0x88);
    msb = I2C_ReadReg(BME280_ADDR, 0x89);
    dig_T1 = (msb << 8) | lsb;

    lsb = I2C_ReadReg(BME280_ADDR, 0x8A);
    msb = I2C_ReadReg(BME280_ADDR, 0x8B);
    dig_T2 = (msb << 8) | lsb;

    lsb = I2C_ReadReg(BME280_ADDR, 0x8C);
    msb = I2C_ReadReg(BME280_ADDR, 0x8D);
    dig_T3 = (msb << 8) | lsb;
}

float BME280_ReadTemperature(void)
{
    int32_t adc_T;
    int32_t var1, var2, T;

    uint8_t msb = I2C_ReadReg(BME280_ADDR, REG_TEMP_MSB);
    uint8_t lsb = I2C_ReadReg(BME280_ADDR, REG_TEMP_MSB+1);
    uint8_t xlsb = I2C_ReadReg(BME280_ADDR, REG_TEMP_MSB+2);

    adc_T = ((int32_t)msb << 12) |
            ((int32_t)lsb << 4) |
            (xlsb >> 4);

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) *
            ((int32_t)dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
            ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;

    T = (t_fine * 5 + 128) >> 8;

    return T / 100.0f;
}
