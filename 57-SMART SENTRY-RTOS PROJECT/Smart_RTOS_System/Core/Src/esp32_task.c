/*
 * esp32_task.c – SMART SENTRY FINAL FIXED
 *
 * ROOT CAUSE OF GARBAGE ON ESP32:
 *   APP_Log() was printing to USART2 (debug) AND sending to xLogQueue.
 *   But the ESP32Task was also transmitting on USART3.
 *   All internal STM32 logs were being received by ESP32 as garbage.
 *
 * FIX:
 *   ESP32_Send() only called for WIFI: and BT: prefixed messages.
 *   ESP32 Arduino firmware filters and ignores non-prefixed lines.
 *   Non-ASCII bytes from ESP32 dropped in STM32 ring buffer.
 *
 * Protocol:
 *   STM32 → ESP32: "WIFI:alert message\n"  or  "BT:alert message\n"
 *   ESP32 → STM32: "CMD:SILENCE\n"  etc.
 *   ESP32 → STM32: "ESP32:READY\n"  on boot
 */

#include "app.h"
#include <string.h>
#include <stdio.h>

#define ESP_RX_BUF   256
#define RECONNECT_MS 30000

static volatile uint8_t  s_rxRing[ESP_RX_BUF];
static volatile uint16_t s_rxHead = 0;
static volatile uint16_t s_rxTail = 0;
static          uint8_t  s_rxByte;
static          uint8_t  s_esp32Ready = 0;

static void ESP32_ArmRx(void)
{ HAL_UART_Receive_IT(&huart3, &s_rxByte, 1); }

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)
    {
        uint16_t next=(s_rxHead+1)%ESP_RX_BUF;
        if(next!=s_rxTail){ s_rxRing[s_rxHead]=s_rxByte; s_rxHead=next; }
        ESP32_ArmRx();
    }
}

static uint8_t ESP32_GetByte(uint8_t *b)
{
    if(s_rxTail==s_rxHead) return 0;
    *b=s_rxRing[s_rxTail];
    s_rxTail=(s_rxTail+1)%ESP_RX_BUF;
    return 1;
}

/* Send ONLY clean ASCII to ESP32 – no debug logs, no emoji */
static void ESP32_SendRaw(const char *str)
{
    HAL_UART_Transmit(&huart3,(uint8_t*)str,(uint16_t)strlen(str),500);
    /* ensure newline */
    if(str[strlen(str)-1]!='\n')
        HAL_UART_Transmit(&huart3,(uint8_t*)"\n",1,50);
}

/* Parse command from ESP32 */
static void ParseLine(const char *line)
{
    if(strlen(line)==0) return;

    APP_Log("[ESP32] <<RECV: %s", line);

    if(strncmp(line,"ESP32:READY",11)==0)
    {
        s_esp32Ready=1;
        APP_Log("[ESP32] Bridge READY. WiFi+BT active.");
        return;
    }
    if(strncmp(line,"ESP32:WIFI_OK",13)==0)
    { APP_Log("[ESP32] WiFi connected."); return; }

    if(strncmp(line,"ESP32:WIFI_FAIL",15)==0)
    { APP_Log("[ESP32] WiFi FAILED – update SSID/PASS in ESP32 firmware."); return; }

    if(strncmp(line,"ESP32:TCP_OK",12)==0)
    { APP_Log("[ESP32] TCP server connected."); return; }

    if(strncmp(line,"ESP32:TCP_FAIL",14)==0)
    { APP_Log("[ESP32] TCP failed – update SERVER_IP in ESP32 firmware."); return; }

    if(strncmp(line,"CMD:",4)==0)
    {
        const char *cmd=line+4;
        APP_Log("[ESP32] Remote CMD: %s", cmd);

        AlertEvent_t ev; ev.timestamp_ms=APP_GetTick();

        if(strncmp(cmd,"SILENCE",7)==0)
        {
            xEventGroupClearBits(xSystemEvents,EVT_ALERT_ACTIVE|EVT_FIRE_ALERT);
            ev.type=ALERT_NONE;
            xQueueSend(xAlertQueue,&ev,0);
            ESP32_SendRaw("BT:SENTRY:SILENCED\n");
            APP_Log("[ESP32] System SILENCED by remote.");
        }
        else if(strncmp(cmd,"LIGHTS_ON",9)==0)
        { ev.type=ALERT_LIGHTS_ON; xQueueSend(xAlertQueue,&ev,0);
          ESP32_SendRaw("BT:SENTRY:LIGHTS_ON\n"); }
        else if(strncmp(cmd,"LIGHTS_OFF",10)==0)
        { ev.type=ALERT_LIGHTS_OFF; xQueueSend(xAlertQueue,&ev,0);
          ESP32_SendRaw("BT:SENTRY:LIGHTS_OFF\n"); }
        else if(strncmp(cmd,"STATUS",6)==0)
        {
            char reply[64];
            uint8_t active=(xEventGroupGetBits(xSystemEvents)&EVT_ALERT_ACTIVE)?1:0;
            snprintf(reply,sizeof(reply),"BT:SENTRY:OK AlertActive=%d\n",active);
            ESP32_SendRaw(reply);
        }
        else
            ESP32_SendRaw("BT:SENTRY:UNKNOWN_CMD\n");
    }
}

void vESP32Task(void *pvParameters)
{
    (void)pvParameters;

    APP_Log("[ESP32 Task] Started. USART3 PB10(TX)/PB11(RX) 115200");
    APP_Log("[ESP32 Task] Wiring: PB10->GPIO16  PB11<-GPIO17");

    ESP32_ArmRx();

    APP_Log("[ESP32 Task] Waiting 8s for ESP32:READY...");
    vTaskDelay(pdMS_TO_TICKS(8000));

    if(!s_esp32Ready)
    {
        APP_Log("[ESP32 Task] No READY signal.");
        APP_Log("[ESP32 Task]  1. Flash ESP32 firmware first");
        APP_Log("[ESP32 Task]  2. Check PB10->GPIO16, PB11<-GPIO17");
        APP_Log("[ESP32 Task]  3. Check ESP32 powered (3.3V+GND)");
        APP_Log("[ESP32 Task]  4. Baud must be 115200 on both sides");
    }

    char    lineBuf[128];
    uint8_t lineIdx=0;
    uint8_t b;

    WirelessMsg_t msg;
    TickType_t    xLastRetry=xTaskGetTickCount();

    for(;;)
    {
        /* WiFi queue → ESP32 with WIFI: prefix */
        if(xQueueReceive(xWifiQueue,&msg,0)==pdTRUE)
        {
            if(s_esp32Ready)
            {
                char out[160];
                snprintf(out,sizeof(out),"WIFI:%s\n",msg.message);
                ESP32_SendRaw(out);
                APP_Log("[ESP32] >>WIFI: %s", msg.message);
            }
        }

        /* BT queue → ESP32 with BT: prefix */
        if(xQueueReceive(xBtQueue,&msg,0)==pdTRUE)
        {
            if(s_esp32Ready)
            {
                char out[160];
                snprintf(out,sizeof(out),"BT:%s\n",msg.message);
                ESP32_SendRaw(out);
                APP_Log("[ESP32] >>BT: %s", msg.message);
            }
        }

        /* RX ring buffer → line parser (ASCII only) */
        while(ESP32_GetByte(&b))
        {
            if(b=='\n'||b=='\r')
            {
                if(lineIdx>0)
                { lineBuf[lineIdx]='\0'; ParseLine(lineBuf); lineIdx=0; }
            }
            else if(b>=0x20 && b<0x7F)   /* printable ASCII only */
            {
                if(lineIdx<sizeof(lineBuf)-1)
                    lineBuf[lineIdx++]=(char)b;
            }
            /* Non-ASCII silently dropped */
        }

        /* Retry message every 30s if offline */
        if(!s_esp32Ready &&
           (xTaskGetTickCount()-xLastRetry)>=pdMS_TO_TICKS(RECONNECT_MS))
        {
            xLastRetry=xTaskGetTickCount();
            APP_Log("[ESP32 Task] Still waiting for ESP32...");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
