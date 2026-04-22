/**
 * @file    alert_task.c
 * @brief   Alert output task – controls LD2 LED (PA5) based on alert type.
 *
 * @details Blocks on xAlertQueue (portMAX_DELAY).  Each alert type produces
 *          a distinct LED pattern so the alert can be identified visually
 *          without a terminal.
 *
 * LED patterns:
 *  - ALERT_FIRE         : rapid 10× blink 50 ms on/off (urgent)
 *  - ALERT_INTRUDER     : 15× blink 100 ms on/off
 *  - ALERT_HUMAN_PRESENT: 2× blink 200 ms on/off
 *  - ALERT_LIGHTS_ON    : LED solid ON
 *  - ALERT_LIGHTS_OFF   : LED solid OFF
 *  - ALERT_POWER_SAVE   : 1× slow blink 50 ms on / 1950 ms off
 *  - ALERT_NONE         : LED OFF
 */

#include "app.h"

#define LED_PORT GPIOA      /**< GPIO port for LD2 */
#define LED_PIN  GPIO_PIN_5 /**< GPIO pin  for LD2 */

/** @brief Turn LD2 on. */
static inline void LED_On(void)
{ HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); }

/** @brief Turn LD2 off. */
static inline void LED_Off(void)
{ HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); }

/**
 * @brief  Blink the LED n times.
 * @param  n       Number of blink cycles.
 * @param  on_ms   LED-on duration in milliseconds.
 * @param  off_ms  LED-off duration in milliseconds.
 */
static void Blink(uint8_t n, uint32_t on_ms, uint32_t off_ms)
{
    for (uint8_t i = 0; i < n; i++)
    {
        LED_On();
        vTaskDelay(pdMS_TO_TICKS(on_ms));
        LED_Off();
        vTaskDelay(pdMS_TO_TICKS(off_ms));
    }
}

/* ══════════════════════════════════════════════════════════════
 * TASK ENTRY
 * ══════════════════════════════════════════════════════════════ */
/**
 * @brief  Alert task entry point.
 *
 * @details Waits indefinitely on xAlertQueue.  On receipt, executes
 *          the LED pattern for the given AlertType_t and logs the event.
 *
 * @param  pvParameters  Unused.
 */
void vAlertTask(void *pvParameters)
{
    (void)pvParameters;

    LED_Off();
    APP_Log("[Alert Task] Started. LED=PA5.");

    AlertEvent_t event;

    for (;;)
    {
        if (xQueueReceive(xAlertQueue, &event, portMAX_DELAY) != pdTRUE)
            continue;

        switch (event.type)
        {
            /* ── FIRE: fastest blink – most urgent ───────────── */
            case ALERT_FIRE:
                APP_Log("[Alert] !! FIRE !! LED rapid blink x10");
                Blink(10, 50, 50);   /* 50 ms on, 50 ms off × 10 = 1 s */
                Blink(10, 50, 50);   /* repeat for 2 s total            */
                break;

            /* ── INTRUDER: fast blink ────────────────────────── */
            case ALERT_INTRUDER:
                APP_Log("[Alert] !! INTRUDER !! LED blink x15");
                Blink(5, 100, 100);
                Blink(5, 100, 100);
                Blink(5, 100, 100);
                break;

            /* ── HUMAN PRESENT: 2 short blinks ──────────────── */
            case ALERT_HUMAN_PRESENT:
                APP_Log("[Alert] Human present – 2 blinks");
                Blink(2, 200, 100);
                break;

            /* ── LIGHTS ON: solid LED ────────────────────────── */
            case ALERT_LIGHTS_ON:
                APP_Log("[Alert] Lights ON");
                LED_On();
                break;

            /* ── LIGHTS OFF: LED off ─────────────────────────── */
            case ALERT_LIGHTS_OFF:
                APP_Log("[Alert] Lights OFF");
                LED_Off();
                break;

            /* ── POWER SAVE: slow heartbeat ──────────────────── */
            case ALERT_POWER_SAVE:
                APP_Log("[Alert] Power save heartbeat");
                Blink(1, 50, 1950);
                break;

            /* ── NONE / default: clear LED ───────────────────── */
            case ALERT_NONE:
            default:
                APP_Log("[Alert] Cleared – LED off");
                LED_Off();
                break;
        }
    }
}
