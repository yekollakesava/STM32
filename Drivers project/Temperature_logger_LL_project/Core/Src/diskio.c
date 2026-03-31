#include "ff.h"
#include "diskio.h"
#include "sdcard.h"
#include <stdio.h>
#include "uart.h"
#include "rtc.h"

#define SD_DRIVE 0

/* ---------------------------------------------------------- */
DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != SD_DRIVE)
        return STA_NOINIT;

    return 0;
}
/* ---------------------------------------------------------- */

DSTATUS disk_initialize (BYTE pdrv)
{
    UART2_Print("disk_initialize\r\n");

    uint8_t result = SD_Init();

    char buf[20];
    sprintf(buf,"SD_Init ret: %d\r\n",result);
    UART2_Print(buf);

    if(result == 0)
        return 0;

    return STA_NOINIT;
}
/* ---------------------------------------------------------- */

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != 0 || count == 0)
        return RES_PARERR;

    for (UINT i = 0; i < count; i++)
    {
        if (SD_ReadBlock(sector + i, buff + (512 * i)) != 0)
            return RES_ERROR;
    }

    return RES_OK;
}
/* ---------------------------------------------------------- */

#if FF_FS_READONLY == 0
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != SD_DRIVE || count == 0)
        return RES_PARERR;

    for (UINT i = 0; i < count; i++)
    {
        if (SD_WriteBlock(sector + i, (uint8_t*)buff + (512 * i)) != 0)
            return RES_ERROR;
    }

    return RES_OK;
}
#endif
/* ---------------------------------------------------------- */

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    if (pdrv != 0)
        return RES_PARERR;

    switch (cmd)
    {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            return RES_OK;

        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 8;   // Typical erase block size
            return RES_OK;

        case GET_SECTOR_COUNT:
            *(DWORD*)buff = 16777216;  // 8GB SDHC
            return RES_OK;

        default:
            return RES_PARERR;
    }
}
/* ---------------------------------------------------------- */
DWORD get_fattime(void)
{
    uint8_t h,m,s,d,mo,y;

    RTC_Get_Time(&h,&m,&s);
    RTC_Get_Date(&d,&mo,&y);   // y = 26

    uint16_t full_year = 2000 + y;   // 2026
    uint16_t fat_year  = full_year - 1980;   // 46

    return  ((DWORD)fat_year << 25)
          | ((DWORD)mo << 21)
          | ((DWORD)d  << 16)
          | ((DWORD)h  << 11)
          | ((DWORD)m  << 5)
          | ((DWORD)(s / 2));
}
