/**
 * @file    decision_task.c
 * @brief   Sensor fusion and alert decision engine.
 *
 * @details Consumes MotionEvent_t from xMotionQueue and the latest
 *          EnvData_t from xEnvQueue, applies a rule set, and posts
 *          AlertEvent_t to xAlertQueue + WirelessMsg_t to xWifiQueue
 *          and xBtQueue.
 *
 * Rule set:
 *  1. Both sensors + dark         → INTRUDER alert
 *  2. Any motion + hot            → DANGER (fire + motion)
 *  3. Any motion + dark           → LIGHTS ON
 *  4. Any motion + normal temp    → HUMAN PRESENT
 *  5. High temp + no motion 5 s   → FIRE alert (periodic check)
 *  6. No motion 30 s              → POWER SAVE
 */

#include "app.h"
#include <string.h>
#include <stdio.h>

static EnvData_t s_env        = {.temperature_c = 0.0f, .light_raw = 200};
static uint32_t  s_lastMotion = 0;
static uint8_t   s_alertActive= 0;

#define NO_MOTION_SAVE_MS  30000UL /**< 30 s idle before power-save */

/* ── Internal helpers ────────────────────────────────────────── */

/**
 * @brief  Post an AlertEvent_t to xAlertQueue.
 * @param  type  Alert classification.
 * @param  desc  Debug description string for UART log.
 */
static void SendAlert(AlertType_t type, const char *desc)
{
    AlertEvent_t alert;
    alert.type         = type;
    alert.timestamp_ms = APP_GetTick();
    if (xQueueSend(xAlertQueue, &alert, pdMS_TO_TICKS(20)) != pdTRUE)
        APP_Log("[Decision] xAlertQueue FULL!");
    APP_Log("[Decision] ALERT --> %s", desc);
}

/**
 * @brief  Post a WirelessMsg_t to both xWifiQueue and xBtQueue.
 * @param  msg   Alert string (max 127 chars).
 * @param  prio  1 = high, 0 = info.
 */
static void Broadcast(const char *msg, uint8_t prio)
{
    WirelessMsg_t w;
    w.priority = prio;
    snprintf(w.message, sizeof(w.message), "%s", msg);
    xQueueSend(xWifiQueue, &w, pdMS_TO_TICKS(10));
    xQueueSend(xBtQueue,   &w, pdMS_TO_TICKS(10));
}

/**
 * @brief  Process a motion event and apply the rule set.
 * @param  m  Pointer to the received MotionEvent_t.
 */
static void ProcessMotion(const MotionEvent_t *m)
{
    s_lastMotion  = m->timestamp_ms;
    s_alertActive = 1;
    xEventGroupClearBits(xSystemEvents, EVT_POWER_SAVE);

    uint8_t both = m->ir_state && m->microwave_state;
    uint8_t any  = m->ir_state || m->microwave_state;
    uint8_t dark = (s_env.light_raw < LIGHT_DARK_THRESHOLD);
    /* Integer threshold compare – avoids float on Cortex-M4 without FPU save */
    int32_t temp100 = (int32_t)(s_env.temperature_c * 100.0f);
    uint8_t hot     = (temp100 >= TEMP_FIRE_THRESHOLD_INT);

    APP_Log("[Decision] IR=%d MW=%d T=%ld.%02ldC Dark=%d Hot=%d",
            m->ir_state, m->microwave_state,
            (long)(temp100/100), (long)(temp100%100),
            dark, hot);

    /* Rule 1: Both sensors + dark = INTRUDER */
    if (both && dark)
    {
        SendAlert(ALERT_INTRUDER, "INTRUDER dual sensor dark room");
        Broadcast("ALERT:INTRUDER detected!", 1);
        xEventGroupSetBits(xSystemEvents, EVT_ALERT_ACTIVE);
        return;
    }

    /* Rule 2: Motion + high temp = DANGER */
    if (any && hot)
    {
        SendAlert(ALERT_INTRUDER, "DANGER motion + high temp");
        Broadcast("ALERT:Motion + high temperature!", 1);
        xEventGroupSetBits(xSystemEvents, EVT_ALERT_ACTIVE);
        return;
    }

    /* Rule 3: Motion + dark = auto lights on */
    if (any && dark)
    {
        SendAlert(ALERT_LIGHTS_ON, "Auto lights ON dark+motion");
        Broadcast("INFO:Lights ON – motion in dark room", 0);
    }

    /* Rule 4: Motion + normal temp = human present */
    if (any && !hot)
    {
        SendAlert(ALERT_HUMAN_PRESENT, "Human present");
        Broadcast("INFO:Human presence detected", 0);
    }

    if (both)
        APP_Log("[Decision] DUAL sensor – high confidence");
}

/**
 * @brief  Periodic checks for slow-onset conditions (fire, power-save).
 *         Called every DECISION_PRINT_INTERVAL_MS (5 s).
 */
static void PeriodicCheck(void)
{
    uint32_t now    = APP_GetTick();
    uint32_t since  = now - s_lastMotion;
    int32_t  temp100= (int32_t)(s_env.temperature_c * 100.0f);
    uint8_t  hot    = (temp100 >= TEMP_FIRE_THRESHOLD_INT);

    APP_Log("[Status] T=%ld.%02ldC L=%u NoMotion=%lus Alert=%d",
            (long)(temp100/100), (long)(temp100%100),
            s_env.light_raw, since/1000, s_alertActive);

    /* Rule 5: High temp + no motion 5 s = FIRE */
    if (hot && since > 5000UL)
    {
        SendAlert(ALERT_FIRE, "FIRE high temp no motion");
        Broadcast("ALERT:FIRE detected!", 1);
        xEventGroupSetBits(xSystemEvents, EVT_FIRE_ALERT);
        return;
    }

    /* Rule 6: No motion 30 s = power save */
    if (s_alertActive && since > NO_MOTION_SAVE_MS)
    {
        APP_Log("[Decision] Power save – no motion %lu s", since/1000);
        SendAlert(ALERT_POWER_SAVE, "Power save no motion 30s");
        SendAlert(ALERT_LIGHTS_OFF, "Lights OFF");
        Broadcast("INFO:Power save mode", 0);
        xEventGroupSetBits(xSystemEvents, EVT_POWER_SAVE);
        xEventGroupClearBits(xSystemEvents, EVT_ALERT_ACTIVE);
        s_alertActive = 0;
    }
}

/* ══════════════════════════════════════════════════════════════
 * TASK ENTRY
 * ══════════════════════════════════════════════════════════════ */
/**
 * @brief  Decision task entry point.
 *
 * @details Drains xEnvQueue (non-blocking) to keep s_env current,
 *          then blocks up to 500 ms on xMotionQueue.  Every 5 s
 *          calls PeriodicCheck() for slow-onset conditions.
 *          500 ms block matches Env task read rate and prevents
 *          starving lower-priority tasks.
 *
 * @param  pvParameters  Unused.
 */
void vDecisionTask(void *pvParameters)
{
    (void)pvParameters;

    APP_Log("[Decision Task] Started. FireThresh=%dC DarkThresh=%u",
            (int)TEMP_FIRE_THRESHOLD_C, LIGHT_DARK_THRESHOLD);

    MotionEvent_t motion;
    EnvData_t     env;

    const TickType_t CHECK_PERIOD = pdMS_TO_TICKS(DECISION_PRINT_INTERVAL_MS);
    TickType_t xLastCheck = xTaskGetTickCount();

    for (;;)
    {
        /* Drain env queue – keep most recent reading */
        while (xQueueReceive(xEnvQueue, &env, 0) == pdTRUE)
            s_env = env;

        /* Block 500 ms waiting for motion – yields CPU to lower tasks */
        if (xQueueReceive(xMotionQueue, &motion, pdMS_TO_TICKS(500)) == pdTRUE)
            ProcessMotion(&motion);

        /* Periodic slow-onset check every 5 s */
        if ((xTaskGetTickCount() - xLastCheck) >= CHECK_PERIOD)
        {
            xLastCheck = xTaskGetTickCount();
            PeriodicCheck();
        }
    }
}
