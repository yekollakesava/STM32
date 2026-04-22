/*
 * logger_task.c – SMART SENTRY
 *
 * FIXES:
 *  - Flush only when page FULL or every 10s (not every 2 entries)
 *  - PAGE_SIZE=256, LOG_ENTRY_SIZE=96 → max 2 entries per page is correct
 *    but was flushing too early. Now waits for full page or timer.
 *  - Reduced log spam during init
 */

#include "app.h"
#include <string.h>
#include <stdio.h>

#define CMD_WEN   0x06
#define CMD_PP    0x02
#define CMD_RSR   0x05
#define CMD_SE    0x20
#define CMD_JEDEC 0x9F

#define LOG_ENTRY_SIZE  96
#define PAGE_SIZE       256
#define LOG_FLUSH_MS    10000

static uint32_t s_addr    = 0x000000;
static uint32_t s_total   = 0;
static uint8_t  s_page[PAGE_SIZE];
static uint16_t s_pageIdx = 0;
static uint8_t  s_flashOk = 0;

static void SPI_Tx(uint8_t *b, uint16_t len)
{ HAL_SPI_Transmit(&hspi2, b, len, 200); }

static void SPI_Rx(uint8_t *b, uint16_t len)
{ HAL_SPI_Receive(&hspi2, b, len, 200); }

static void Flash_WaitBusy(void)
{
    uint8_t cmd=CMD_RSR, s;
    FLASH_CS_LOW(); SPI_Tx(&cmd,1);
    uint32_t t=HAL_GetTick();
    do { SPI_Rx(&s,1);
         if(HAL_GetTick()-t>600){ APP_Log("[Logger] WaitBusy TIMEOUT"); break; }
    } while(s&0x01);
    FLASH_CS_HIGH();
}

static void Flash_WEN(void)
{ uint8_t c=CMD_WEN; FLASH_CS_LOW(); SPI_Tx(&c,1); FLASH_CS_HIGH(); }

static uint8_t Flash_Init(void)
{
    uint8_t cmd=CMD_JEDEC, id[3]={0};
    FLASH_CS_LOW(); SPI_Tx(&cmd,1); SPI_Rx(id,3); FLASH_CS_HIGH();
    APP_Log("[Logger] JEDEC: %02X %02X %02X", id[0],id[1],id[2]);
    if(id[0]==0xEF){ APP_Log("[Logger] W25Q Flash OK (Winbond)"); return 1; }
    if(id[0]==0xFF||id[0]==0x00)
    { APP_Log("[Logger] Flash NOT detected – check PB12-15 wiring"); return 0; }
    APP_Log("[Logger] Unknown MFR 0x%02X", id[0]);
    return 1;
}

static void Flash_Erase(uint32_t addr)
{
    Flash_WEN();
    uint8_t cmd[4]={CMD_SE,(uint8_t)(addr>>16),(uint8_t)(addr>>8),(uint8_t)addr};
    FLASH_CS_LOW(); SPI_Tx(cmd,4); FLASH_CS_HIGH();
    Flash_WaitBusy();
}

static void Flash_WritePage(uint32_t addr, uint8_t *data, uint16_t len)
{
    Flash_WEN();
    uint8_t hdr[4]={CMD_PP,(uint8_t)(addr>>16),(uint8_t)(addr>>8),(uint8_t)addr};
    FLASH_CS_LOW(); SPI_Tx(hdr,4); SPI_Tx(data,len); FLASH_CS_HIGH();
    Flash_WaitBusy();
}

static void FlushPage(void)
{
    if(s_pageIdx==0) return;   /* nothing to flush */

    if((s_addr%4096)==0)
        Flash_Erase(s_addr);

    if(xSemaphoreTake(xSpiMutex,pdMS_TO_TICKS(500))==pdTRUE)
    {
        Flash_WritePage(s_addr, s_page, PAGE_SIZE);
        xSemaphoreGive(xSpiMutex);
        uint32_t entries = s_pageIdx / LOG_ENTRY_SIZE;
        APP_Log("[Logger] Page written: %lu entries at 0x%06lX", entries, s_addr);
    }
    else
        APP_Log("[Logger] SPI mutex timeout");

    s_addr   += PAGE_SIZE;
    s_pageIdx = 0;
    memset(s_page, 0xFF, PAGE_SIZE);
}

static void BufferLog(const LogEntry_t *e)
{
    /* If this entry won't fit, flush first */
    if(s_pageIdx + LOG_ENTRY_SIZE > PAGE_SIZE)
        FlushPage();

    uint8_t packed[LOG_ENTRY_SIZE];
    memset(packed,0xFF,LOG_ENTRY_SIZE);
    packed[0]=(uint8_t)(e->timestamp_ms>>24);
    packed[1]=(uint8_t)(e->timestamp_ms>>16);
    packed[2]=(uint8_t)(e->timestamp_ms>>8);
    packed[3]=(uint8_t)(e->timestamp_ms);
    strncpy((char*)&packed[4], e->log, LOG_ENTRY_SIZE-5);
    packed[LOG_ENTRY_SIZE-1]='\0';

    memcpy(&s_page[s_pageIdx], packed, LOG_ENTRY_SIZE);
    s_pageIdx += LOG_ENTRY_SIZE;
    s_total++;
}

void vLoggerTask(void *pvParameters)
{
    (void)pvParameters;
    APP_Log("[Logger Task] Started. SPI2 PB12/PB13/PB14/PB15");

    memset(s_page,0xFF,PAGE_SIZE);

    if(xSemaphoreTake(xSpiMutex,pdMS_TO_TICKS(1000))==pdTRUE)
    { s_flashOk=Flash_Init(); xSemaphoreGive(xSpiMutex); }

    if(s_flashOk)
        APP_Log("[Logger Task] Flash READY. Flush every 10s or when page full.");
    else
        APP_Log("[Logger Task] No flash – queue drain only.");

    LogEntry_t    entry;
    TickType_t    xLastFlush = xTaskGetTickCount();
    uint32_t      qCount     = 0;

    for(;;)
    {
        /* Drain queue with 200ms block */
        if(xQueueReceive(xLogQueue, &entry, pdMS_TO_TICKS(200))==pdTRUE)
        {
            qCount++;
            if(s_flashOk)
                BufferLog(&entry);
        }

        /* Flush only on timer */
        if((xTaskGetTickCount()-xLastFlush) >= pdMS_TO_TICKS(LOG_FLUSH_MS))
        {
            xLastFlush=xTaskGetTickCount();
            if(s_flashOk && s_pageIdx>0)
            {
                APP_Log("[Logger] Timer flush. Total stored: %lu", s_total);
                FlushPage();
            }
        }
    }
}
