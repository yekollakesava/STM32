/**
 * @file    env_task.c
 * @brief   Environmental sensor task – BME280 temperature + OPT3001 light.
 *
 * @details Reads BME280 and OPT3001 every 500 ms using pure integer arithmetic
 *          (no float in the sensor path, safe with configENABLE_FPU=0).
 *          Posts EnvData_t to xEnvQueue via xQueueOverwrite (queue depth=1).
 *          Sends immediate ALERT_FIRE to xAlertQueue AND broadcasts via
 *          WiFi+BT the moment temperature crosses TEMP_FIRE_THRESHOLD_INT.
 *          Periodic status printed every 3 s (6 reads × 500 ms).
 *
 * @note    All local buffers are static to avoid stack pressure.
 *          Uses vTaskDelay (not vTaskDelayUntil) to guarantee a yield
 *          even when I2C timeouts cause iteration to exceed 500 ms.
 */

#include "app.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef  hi2c1;
extern UART_HandleTypeDef huart2;

/** @defgroup BME280Regs BME280 register addresses */
/** @{ */
#define BME280_ADDR   (0x77 << 1) /**< I2C address (SDO=VCC)   */
#define BME_CHIPID    0xD0
#define BME_RESET     0xE0
#define BME_CTRL_HUM  0xF2
#define BME_CTRL_MEAS 0xF4
#define BME_CONFIG    0xF5
#define BME_TEMP_MSB  0xFA
#define BME_CALIB00   0x88
/** @} */

/** @defgroup OPT3001Regs OPT3001 register addresses */
/** @{ */
#define OPT3001_ADDR  (0x47 << 1) /**< I2C address (ADDR=GND)  */
#define OPT_RESULT    0x00
#define OPT_CONFIG    0x01
/** @} */

#define PRINT_EVERY_N_READS  6    /**< 6 × 500 ms = 3 s between prints */

/* ── Module-level state ──────────────────────────────────────── */
static uint16_t s_dig_T1;        /**< BME280 calibration T1             */
static int16_t  s_dig_T2;        /**< BME280 calibration T2             */
static int16_t  s_dig_T3;        /**< BME280 calibration T3             */
static int32_t  s_t_fine;        /**< BME280 intermediate fine value    */
static uint8_t  s_bme_ok = 0;    /**< 1 when BME280 initialised OK      */
static uint8_t  s_opt_ok = 0;    /**< 1 when OPT3001 initialised OK     */

/* Static buffers – live in BSS, NOT on the task stack */
static char    s_printbuf[80];
static uint8_t s_i2c_buf[8];

/* ── Crash-proof direct UART print ──────────────────────────── */
/**
 * @brief  Print a string directly to huart2 without using the mutex.
 *         Used inside the sensor task where stack is tight.
 * @param  s  Null-terminated string to transmit.
 */
static void P(const char *s)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), 300);
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 50);
}

/* ── Safe I2C helpers ────────────────────────────────────────── */
/**
 * @brief  Write one byte to an I2C device register.
 * @param  dev  7-bit I2C address shifted left by 1.
 * @param  reg  Register address.
 * @param  val  Byte to write.
 */
static void I2C_Write(uint8_t dev, uint8_t reg, uint8_t val)
{
    s_i2c_buf[0] = reg;
    s_i2c_buf[1] = val;
    HAL_I2C_Master_Transmit(&hi2c1, dev, s_i2c_buf, 2, 50);
}

/**
 * @brief  Read one byte from an I2C device register.
 * @param  dev  7-bit I2C address shifted left by 1.
 * @param  reg  Register address.
 * @return Byte read, or 0 on error.
 */
static uint8_t I2C_Read1(uint8_t dev, uint8_t reg)
{
    s_i2c_buf[0] = reg;
    if (HAL_I2C_Master_Transmit(&hi2c1, dev, s_i2c_buf, 1, 50) != HAL_OK) return 0;
    if (HAL_I2C_Master_Receive (&hi2c1, dev, s_i2c_buf, 1, 50) != HAL_OK) return 0;
    return s_i2c_buf[0];
}

/**
 * @brief  Read N bytes from an I2C device starting at reg.
 * @param  dev  7-bit I2C address shifted left by 1.
 * @param  reg  Starting register address.
 * @param  buf  Destination buffer (caller-allocated).
 * @param  len  Number of bytes to read.
 * @return HAL_OK on success, HAL_ERROR on bus failure.
 */
static HAL_StatusTypeDef I2C_ReadN(uint8_t dev, uint8_t reg,
                                    uint8_t *buf, uint8_t len)
{
    s_i2c_buf[0] = reg;
    if (HAL_I2C_Master_Transmit(&hi2c1, dev, s_i2c_buf, 1, 50) != HAL_OK)
        return HAL_ERROR;
    return HAL_I2C_Master_Receive(&hi2c1, dev, buf, len, 100);
}

/* ── Wireless broadcast helper ───────────────────────────────── */
/**
 * @brief  Post a message to both xWifiQueue and xBtQueue.
 * @param  msg   Null-terminated alert string (max 127 chars).
 * @param  prio  1 = high priority alert, 0 = informational.
 */
static void Broadcast(const char *msg, uint8_t prio)
{
    WirelessMsg_t w;
    w.priority = prio;
    snprintf(w.message, sizeof(w.message), "%s", msg);
    xQueueSend(xWifiQueue, &w, pdMS_TO_TICKS(10));
    xQueueSend(xBtQueue,   &w, pdMS_TO_TICKS(10));
}

/* ── Send alert to LED task ──────────────────────────────────── */
/**
 * @brief  Post an AlertEvent_t to xAlertQueue for vAlertTask.
 * @param  type  Alert classification from AlertType_t.
 */
static void SendAlert(AlertType_t type)
{
    AlertEvent_t ev;
    ev.type         = type;
    ev.timestamp_ms = APP_GetTick();
    xQueueSend(xAlertQueue, &ev, pdMS_TO_TICKS(20));
}

/* ══════════════════════════════════════════════════════════════
 * BME280
 * ══════════════════════════════════════════════════════════════ */
/**
 * @brief  Initialise BME280 in normal mode (forced, 1x oversampling).
 *         Reads and stores temperature calibration coefficients T1–T3.
 *         Sets s_bme_ok=1 on success.
 */
static void BME280_Init(void)
{
    static uint8_t cal[6];
    P("[BME280] Init...");
    if (HAL_I2C_IsDeviceReady(&hi2c1, BME280_ADDR, 2, 100) != HAL_OK)
        { P("[BME280] NOT FOUND"); return; }
    if (I2C_Read1(BME280_ADDR, BME_CHIPID) != 0x60)
        { P("[BME280] Wrong chip ID"); return; }

    I2C_Write(BME280_ADDR, BME_RESET,     0xB6);
    vTaskDelay(pdMS_TO_TICKS(10));
    I2C_Write(BME280_ADDR, BME_CTRL_HUM,  0x01);
    I2C_Write(BME280_ADDR, BME_CTRL_MEAS, 0x27);
    I2C_Write(BME280_ADDR, BME_CONFIG,    0xA0);
    vTaskDelay(pdMS_TO_TICKS(100));

    if (I2C_ReadN(BME280_ADDR, BME_CALIB00, cal, 6) != HAL_OK)
        { P("[BME280] Calib fail"); return; }

    s_dig_T1 = (uint16_t)((cal[1]<<8)|cal[0]);
    s_dig_T2 = (int16_t) ((cal[3]<<8)|cal[2]);
    s_dig_T3 = (int16_t) ((cal[5]<<8)|cal[4]);
    s_bme_ok  = 1;
    P("[BME280] READY");
}

/**
 * @brief  Read compensated temperature from BME280.
 * @return Temperature × 100 as int32_t.  e.g. 2531 = 25.31 °C.
 *         Returns 0 if sensor not ready or I2C error.
 *         Zero float arithmetic used – safe with configENABLE_FPU=0.
 */
static int32_t BME280_ReadTemp_Int(void)
{
    static uint8_t r[3];
    if (!s_bme_ok) return 0;
    if (I2C_ReadN(BME280_ADDR, BME_TEMP_MSB, r, 3) != HAL_OK) return 0;
    int32_t adc = ((int32_t)r[0]<<12)|((int32_t)r[1]<<4)|(r[2]>>4);
    int32_t v1  = ((((adc>>3)-((int32_t)s_dig_T1<<1)))*(int32_t)s_dig_T2)>>11;
    int32_t v2  = (((((adc>>4)-(int32_t)s_dig_T1)*((adc>>4)-(int32_t)s_dig_T1))>>12)
                   *(int32_t)s_dig_T3)>>14;
    s_t_fine = v1 + v2;
    return (s_t_fine * 5 + 128) >> 8;  /* = temperature * 100 */
}

/* ══════════════════════════════════════════════════════════════
 * OPT3001
 * ══════════════════════════════════════════════════════════════ */
/**
 * @brief  Initialise OPT3001 in continuous conversion mode.
 *         Sets s_opt_ok=1 on success.
 */
static void OPT3001_Init(void)
{
    static uint8_t cfg[3];
    P("[OPT3001] Init...");
    if (HAL_I2C_IsDeviceReady(&hi2c1, OPT3001_ADDR, 2, 100) != HAL_OK)
        { P("[OPT3001] NOT FOUND"); return; }
    cfg[0]=OPT_CONFIG; cfg[1]=0xCC; cfg[2]=0x10;
    HAL_I2C_Master_Transmit(&hi2c1, OPT3001_ADDR, cfg, 3, 50);
    vTaskDelay(pdMS_TO_TICKS(110));
    s_opt_ok = 1;
    P("[OPT3001] READY");
}

/**
 * @brief  Read lux value from OPT3001.
 * @return Lux × 10 as uint16_t.  e.g. 253 = 25.3 lux.
 *         Returns 0 if sensor not ready or I2C error.
 *         Zero float arithmetic used.
 */
static uint16_t OPT3001_ReadLux_Int(void)
{
    static uint8_t r[2];
    static uint8_t reg;
    if (!s_opt_ok) return 0;
    reg = OPT_RESULT;
    if (HAL_I2C_Master_Transmit(&hi2c1, OPT3001_ADDR, &reg, 1, 50)  != HAL_OK) return 0;
    if (HAL_I2C_Master_Receive (&hi2c1, OPT3001_ADDR, r,   2, 100) != HAL_OK) return 0;
    uint16_t raw   = (uint16_t)((r[0]<<8)|r[1]);
    uint32_t lux10 = ((uint32_t)(raw&0x0FFF) * (1u<<((raw>>12)&0xF))) / 10u;
    return (lux10 > 0xFFFF) ? 0xFFFF : (uint16_t)lux10;
}

/* ══════════════════════════════════════════════════════════════
 * TASK ENTRY
 * ══════════════════════════════════════════════════════════════ */
/**
 * @brief  Environmental sensor task entry point.
 *
 * @details Initialises BME280 and OPT3001, then loops every 500 ms:
 *          - Reads temperature (integer × 100) and light (lux × 10).
 *          - Posts to xEnvQueue via xQueueOverwrite.
 *          - On fire threshold crossing: sends ALERT_FIRE to xAlertQueue
 *            AND broadcasts to WiFi + BT immediately.
 *          - On dark threshold crossing: broadcasts immediately.
 *          - Prints a compact status line every 3 s.
 *
 * @param  pvParameters  Unused FreeRTOS parameter.
 */
void vEnvTask(void *pvParameters)
{
    (void)pvParameters;

    P("[Env Task] Started.");
    vTaskDelay(pdMS_TO_TICKS(2000));

    BME280_Init();
    OPT3001_Init();

    P("[Env Task] Loop OK");

    uint8_t  prev_dark  = 2; /**< 2 = unknown, forces first evaluation */
    uint8_t  prev_fire  = 2;
    uint32_t readCount  = 0;
    uint32_t printCount = 0;

    for (;;)
    {
        /* Always yields 500 ms – never spins like vTaskDelayUntil can */
        vTaskDelay(pdMS_TO_TICKS(500));

        readCount++;
        printCount++;

        /* ── Read sensors (integer only, zero float) ─────────── */
        int32_t  temp100 = BME280_ReadTemp_Int(); /* e.g. 2531 = 25.31 C */
        uint16_t lraw    = OPT3001_ReadLux_Int(); /* e.g.  253 = 25.3 lux*/

        /* Integer parts for formatted print */
        int32_t  ti = temp100 / 100;
        int32_t  tf = temp100 % 100; if (tf < 0) tf = -tf;
        uint16_t li = lraw / 10;
        uint16_t lf = lraw % 10;

        /* ── Push to decision task ───────────────────────────── */
        EnvData_t d;
        d.temperature_c = (float)temp100 / 100.0f;
        d.light_raw     = lraw;
        d.timestamp_ms  = APP_GetTick();
        xQueueOverwrite(xEnvQueue, &d);

        /* ── Threshold evaluation (integer compares only) ───── */
        uint8_t is_dark = (lraw    <  LIGHT_DARK_THRESHOLD)    ? 1 : 0;
        uint8_t is_fire = (temp100 >= TEMP_FIRE_THRESHOLD_INT) ? 1 : 0;

        /* ── Fire threshold transition ───────────────────────── */
        if (is_fire != prev_fire)
        {
            prev_fire = is_fire;
            if (is_fire)
            {
                /* Send to LED task AND broadcast wirelessly */
                SendAlert(ALERT_FIRE);
                snprintf(s_printbuf, sizeof(s_printbuf),
                         "ALERT:FIRE %ld.%02ldC >= %dC threshold!",
                         ti, tf, (int)TEMP_FIRE_THRESHOLD_C);
                Broadcast(s_printbuf, 1);
                P("!! [SENSOR] FIRE TEMPERATURE ALERT !!");
                snprintf(s_printbuf, sizeof(s_printbuf),
                         "[SENSOR] Temp=%ld.%02ldC FIRE threshold=%dC",
                         ti, tf, (int)TEMP_FIRE_THRESHOLD_C);
                P(s_printbuf);
                xEventGroupSetBits(xSystemEvents, EVT_FIRE_ALERT);
            }
            else
            {
                SendAlert(ALERT_NONE);
                snprintf(s_printbuf, sizeof(s_printbuf),
                         "INFO:Temperature normal %ld.%02ldC", ti, tf);
                Broadcast(s_printbuf, 0);
                P("[SENSOR] Temp back to normal.");
                xEventGroupClearBits(xSystemEvents, EVT_FIRE_ALERT);
            }
        }

        /* ── Dark threshold transition ───────────────────────── */
        if (is_dark != prev_dark)
        {
            prev_dark = is_dark;
            if (is_dark)
            {
                P("[SENSOR] DARK ROOM DETECTED");
                Broadcast("ALERT:Dark room – motion sensors armed", 1);
            }
            else
            {
                P("[SENSOR] Room is bright.");
                Broadcast("INFO:Room bright", 0);
            }
        }

        /* ── Periodic status print every 3 s ────────────────── */
        if (printCount >= PRINT_EVERY_N_READS)
        {
            printCount = 0;
            snprintf(s_printbuf, sizeof(s_printbuf),
                     "[ENV] T=%ld.%02ldC(%s) L=%u.%ulux(%s) #%lu",
                     ti, tf, is_fire ? "FIRE":"OK",
                     li, lf, is_dark ? "DARK":"bright",
                     readCount);
            P(s_printbuf);
        }
    }
}
