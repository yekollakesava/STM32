#ifndef DISKIO_H
#define DISKIO_H

#include <stdint.h>

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t UINT;

typedef uint8_t DSTATUS;

typedef enum
{
    RES_OK = 0,
    RES_ERROR,
    RES_WRPRT,
    RES_NOTRDY,
    RES_PARERR
} DRESULT;

#define STA_NOINIT      0x01

#define CTRL_SYNC           0
#define GET_SECTOR_COUNT    1
#define GET_SECTOR_SIZE     2
#define GET_BLOCK_SIZE      3

DSTATUS disk_initialize(BYTE pdrv);
DSTATUS disk_status(BYTE pdrv);
DRESULT disk_read(BYTE pdrv, BYTE *buff, uint32_t sector, UINT count);
DRESULT disk_write(BYTE pdrv, const BYTE *buff, uint32_t sector, UINT count);
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);

#endif
