/*
 * motion_task.c – SMART SENTRY FINAL FIXED
 *
 * IR Sensor  → PA0 EXTI0
 * Microwave  → PA1 EXTI1
 *
 * FIX: IR shows HIGH at boot → add 1s settle delay before processing.
 * FIX: Clear detection message with sensor name.
 */

#include "app.h"

static uint32_t s_lastIrTime = 0;
static uint32_t s_lastMwTime = 0;

void vMotionTask(void *pvParameters)
{
    (void)pvParameters;

    APP_Log("[Motion Task] Started.");
    APP_Log("[Motion Task] IR Sensor  : PA0 (EXTI0)");
    APP_Log("[Motion Task] Microwave  : PA1 (EXTI1)");
    APP_Log("[Motion Task] Debounce   : %d ms", MOTION_DEBOUNCE_MS);

    /* Wait 1s for sensors to settle after power-on */
    /* IR sensor output can be HIGH briefly at boot */
    APP_Log("[Motion Task] Settling sensors for 1s...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    APP_Log("[Motion Task] Ready. Waiting for triggers...");

    /* Clear any spurious events that came in during settle */
    xEventGroupClearBits(xSystemEvents,
                         EVT_IR_TRIGGERED | EVT_MICROWAVE_TRIGGERED);

    const EventBits_t WAIT_BITS =
        EVT_IR_TRIGGERED | EVT_MICROWAVE_TRIGGERED;

    for(;;)
    {
        EventBits_t bits = xEventGroupWaitBits(
            xSystemEvents, WAIT_BITS,
            pdTRUE, pdFALSE, portMAX_DELAY);

        uint32_t now = APP_GetTick();

        MotionEvent_t evt = {.ir_state=0, .microwave_state=0,
                             .timestamp_ms=now};

        /* IR sensor */
        if(bits & EVT_IR_TRIGGERED)
        {
            if((now - s_lastIrTime) >= MOTION_DEBOUNCE_MS)
            {
                s_lastIrTime = now;
                evt.ir_state = 1;
                APP_Log("**********************************************");
                APP_Log("*   OBJECT DETECTED – IR Sensor (PA0)       *");
                APP_Log("*   Time: %lu ms                            *", now);
                APP_Log("**********************************************");
            }
        }

        /* Microwave sensor */
        if(bits & EVT_MICROWAVE_TRIGGERED)
        {
            if((now - s_lastMwTime) >= MOTION_DEBOUNCE_MS)
            {
                s_lastMwTime        = now;
                evt.microwave_state = 1;
                APP_Log("**********************************************");
                APP_Log("*   OBJECT DETECTED – Microwave (PA1)       *");
                APP_Log("*   Time: %lu ms                            *", now);
                APP_Log("**********************************************");
            }
        }

        /* Send to Decision Task */
        if(evt.ir_state || evt.microwave_state)
        {
            if(evt.ir_state && evt.microwave_state)
            {
                APP_Log(">>> DUAL SENSOR – HIGH CONFIDENCE DETECTION <<<");
                xEventGroupSetBits(xSystemEvents, EVT_MOTION_CONFIRMED);
            }
            xQueueSend(xMotionQueue, &evt, pdMS_TO_TICKS(20));
        }
    }
}
