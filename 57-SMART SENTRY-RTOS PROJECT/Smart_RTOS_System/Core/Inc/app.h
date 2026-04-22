/**
 * @file    app.h
 * @brief   Smart Sentry – shared definitions, handles, and prototypes.
 *
 * @details Central header for the Smart Sentry security system running on
 *          STM32L476RG with FreeRTOS.  Every task source file includes
 *          only this header.
 *
 * Hardware:
 *  - STM32L476RG Nucleo-64
 *  - BME280  temperature sensor  (I2C1, 0x77)
 *  - OPT3001 light sensor        (I2C1, 0x47)
 *  - W25Q    SPI flash           (SPI2, PB12 CS)  – optional
 *  - ESP32   WiFi + BT bridge    (USART3, 115200)
 *  - IR sensor                   (PA0, EXTI0)
 *  - Microwave sensor            (PA1, EXTI1)
 *  - LD2 LED                     (PA5)
 */

#ifndef APP_H
#define APP_H

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"

/* ══════════════════════════════════════════════════════════════
 * @defgroup EventBits  System event group bit definitions
 * @{
 * ══════════════════════════════════════════════════════════════ */
#define EVT_IR_TRIGGERED        (1 << 0) /**< IR sensor fired          */
#define EVT_MICROWAVE_TRIGGERED (1 << 1) /**< Microwave sensor fired   */
#define EVT_MOTION_CONFIRMED    (1 << 2) /**< Both sensors confirmed   */
#define EVT_ALERT_ACTIVE        (1 << 3) /**< An alert is in progress  */
#define EVT_POWER_SAVE          (1 << 4) /**< System in power-save     */
#define EVT_FIRE_ALERT          (1 << 5) /**< Fire condition active    */
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup MsgStructs  Inter-task message structures
 * @{
 * ══════════════════════════════════════════════════════════════ */

/** @brief Motion event posted by vMotionTask to xMotionQueue. */
typedef struct {
    uint8_t  ir_state;        /**< 1 = IR triggered          */
    uint8_t  microwave_state; /**< 1 = Microwave triggered   */
    uint32_t timestamp_ms;    /**< xTaskGetTickCount() value */
} MotionEvent_t;

/** @brief Environmental snapshot posted by vEnvTask to xEnvQueue. */
typedef struct {
    float    temperature_c;   /**< Temperature in degrees C  */
    uint16_t light_raw;       /**< Lux * 10 (253 = 25.3 lux)*/
    uint32_t timestamp_ms;    /**< Timestamp in ms           */
} EnvData_t;

/** @brief Alert type enumeration used in AlertEvent_t. */
typedef enum {
    ALERT_NONE = 0,      /**< No alert – system normal     */
    ALERT_INTRUDER,      /**< Intruder detected            */
    ALERT_FIRE,          /**< Fire / high temperature      */
    ALERT_HUMAN_PRESENT, /**< Human presence (non-threat)  */
    ALERT_LIGHTS_ON,     /**< Auto lights turned on        */
    ALERT_LIGHTS_OFF,    /**< Lights turned off            */
    ALERT_POWER_SAVE     /**< Power-save mode activated    */
} AlertType_t;

/** @brief Alert event posted to xAlertQueue for vAlertTask. */
typedef struct {
    AlertType_t type;         /**< Alert classification      */
    uint32_t    timestamp_ms; /**< Timestamp in ms           */
} AlertEvent_t;

/** @brief Wireless message sent to xWifiQueue or xBtQueue. */
typedef struct {
    char    message[128]; /**< Null-terminated alert string */
    uint8_t priority;     /**< 1 = high, 0 = info           */
} WirelessMsg_t;

/** @brief Flash log entry sent to xLogQueue for vLoggerTask. */
typedef struct {
    char     log[96];         /**< Null-terminated log string */
    uint32_t timestamp_ms;    /**< Timestamp in ms            */
} LogEntry_t;
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup Handles  Peripheral and RTOS handles (defined in main.c)
 * @{
 * ══════════════════════════════════════════════════════════════ */
extern UART_HandleTypeDef huart1; /**< Future use                  */
extern UART_HandleTypeDef huart2; /**< RealTerm debug  (9600 baud) */
extern UART_HandleTypeDef huart3; /**< ESP32 bridge (115200 baud)  */
extern ADC_HandleTypeDef  hadc1;
extern SPI_HandleTypeDef  hspi2;  /**< W25Q SPI flash              */
extern I2C_HandleTypeDef  hi2c1;  /**< BME280 + OPT3001            */

extern QueueHandle_t      xMotionQueue; /**< MotionEvent_t  depth 4  */
extern QueueHandle_t      xEnvQueue;    /**< EnvData_t      depth 1  */
extern QueueHandle_t      xAlertQueue;  /**< AlertEvent_t   depth 8  */
extern QueueHandle_t      xWifiQueue;   /**< WirelessMsg_t  depth 8  */
extern QueueHandle_t      xBtQueue;     /**< WirelessMsg_t  depth 8  */
extern QueueHandle_t      xLogQueue;    /**< LogEntry_t     depth 10 */
extern SemaphoreHandle_t  xUartMutex;
extern SemaphoreHandle_t  xSpiMutex;
extern SemaphoreHandle_t  xAdcMutex;
extern EventGroupHandle_t xSystemEvents;
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup Priorities  FreeRTOS task priorities
 *
 * Priority ladder (highest number wins CPU):
 *   Motion   6 – must service EXTI immediately
 *   Env      5 – sensor reads must never be starved
 *   Decision 3 – sensor fusion
 *   Alert    3 – LED patterns
 *   ESP32    2 – UART bridge
 *   Logger   1 – flash writes, lowest acceptable
 * @{
 * ══════════════════════════════════════════════════════════════ */
#define PRIORITY_MOTION    (tskIDLE_PRIORITY + 6)
#define PRIORITY_ENV       (tskIDLE_PRIORITY + 5)
#define PRIORITY_DECISION  (tskIDLE_PRIORITY + 3)
#define PRIORITY_ALERT     (tskIDLE_PRIORITY + 3)
#define PRIORITY_ESP32     (tskIDLE_PRIORITY + 2)
#define PRIORITY_LOGGER    (tskIDLE_PRIORITY + 1)
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup StackSizes  Task stack sizes in words (1 word = 4 bytes)
 * @{
 * ══════════════════════════════════════════════════════════════ */
#define STACK_MOTION    300
#define STACK_DECISION  400
#define STACK_ALERT     300
#define STACK_ESP32     512
#define STACK_ENV       700  /**< Extra room for I2C HAL + static bufs */
#define STACK_LOGGER    600
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup Thresholds  Detection thresholds
 * @{
 * ══════════════════════════════════════════════════════════════ */
/** @brief Fire alert threshold in degrees C.
 *  Set to 28.0 so alert triggers at room temperature for testing. */
#define TEMP_FIRE_THRESHOLD_C   28.0f

/** @brief Fire threshold as integer (temp * 100) for integer compare. */
#define TEMP_FIRE_THRESHOLD_INT 2800

/** @brief Light threshold: below 10.0 lux = dark (lux * 10 = 100). */
#define LIGHT_DARK_THRESHOLD    100

/** @brief Debounce time for motion sensors in ms. */
#define MOTION_DEBOUNCE_MS      200
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup Intervals  Periodic task intervals
 * @{
 * ══════════════════════════════════════════════════════════════ */
#define ENV_PRINT_INTERVAL_MS       3000 /**< Env status print period   */
#define DECISION_PRINT_INTERVAL_MS  5000 /**< Decision status period    */
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup FlashCS  SPI flash chip-select macros
 * @{
 * ══════════════════════════════════════════════════════════════ */
#ifndef FLASH_CS_LOW
#define FLASH_CS_LOW()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define FLASH_CS_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#endif
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup TaskProtos  Task entry-point prototypes
 * @{
 * ══════════════════════════════════════════════════════════════ */
void vMotionTask   (void *pvParameters); /**< IR + microwave detection  */
void vDecisionTask (void *pvParameters); /**< Sensor fusion + rules     */
void vAlertTask    (void *pvParameters); /**< LED patterns              */
void vESP32Task    (void *pvParameters); /**< WiFi + BT bridge          */
void vEnvTask      (void *pvParameters); /**< BME280 + OPT3001 reads    */
void vLoggerTask   (void *pvParameters); /**< SPI flash logging         */
/** @} */

/* ══════════════════════════════════════════════════════════════
 * @defgroup Utility  Utility functions
 * @{
 * ══════════════════════════════════════════════════════════════ */
/**
 * @brief  Thread-safe formatted UART print to huart2.
 * @param  fmt  printf-style format string.
 */
void APP_Log(const char *fmt, ...);

/**
 * @brief  Returns current FreeRTOS tick count in ms.
 * @return Tick count cast to uint32_t.
 */
uint32_t APP_GetTick(void);
/** @} */

#endif /* APP_H */
