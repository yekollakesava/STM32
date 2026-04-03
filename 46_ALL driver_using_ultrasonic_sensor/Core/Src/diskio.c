#include "ff_gen_drv.h"
#include "sdcard.h"

static volatile DSTATUS Stat = STA_NOINIT;

DSTATUS USER_initialize(BYTE pdrv)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }

    if (SD_Init() == 0U)
    {
        Stat &= ~STA_NOINIT;
    }
    else
    {
        Stat = STA_NOINIT;
    }

    return Stat;
}

DSTATUS USER_status(BYTE pdrv)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }

    return Stat;
}

DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if ((pdrv != 0U) || (count != 1U))
    {
        return RES_PARERR;
    }

    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    if (SD_ReadBlock(buff, sector) == 0U)
    {
        return RES_OK;
    }

    return RES_ERROR;
}

#if FF_FS_READONLY == 0
DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    if ((pdrv != 0U) || (count != 1U))
    {
        return RES_PARERR;
    }

    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    if (SD_WriteBlock(buff, sector) == 0U)
    {
        return RES_OK;
    }

    return RES_ERROR;
}
#endif

DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    if (pdrv != 0U)
    {
        return RES_PARERR;
    }

    switch (cmd)
    {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_COUNT:
            *(DWORD *)buff = 32768U;
            return RES_OK;

        case GET_SECTOR_SIZE:
            *(WORD *)buff = 512U;
            return RES_OK;

        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 1U;
            return RES_OK;

        default:
            return RES_PARERR;
    }
}

Diskio_drvTypeDef USER_Driver =
{
    USER_initialize,
    USER_status,
    USER_read,
#if FF_FS_READONLY == 0
    USER_write,
#endif
    USER_ioctl
};
