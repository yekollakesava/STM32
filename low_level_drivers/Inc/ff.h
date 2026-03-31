/*
 * ff.h
 *
 *  Created on: Mar 25, 2026
 *      Author: yekol
 */

#ifndef FF_H
#define FF_H

#include <stdint.h>

/* Basic FatFs-like type definitions */
typedef unsigned int    UINT;
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

/* Result type */
typedef enum
{
    FR_OK = 0,
    FR_DISK_ERR,
    FR_INT_ERR,
    FR_NOT_READY,
    FR_NO_FILE,
    FR_NO_PATH,
    FR_INVALID_NAME,
    FR_DENIED,
    FR_EXIST,
    FR_INVALID_OBJECT,
    FR_WRITE_PROTECTED,
    FR_INVALID_DRIVE,
    FR_NOT_ENABLED,
    FR_NO_FILESYSTEM,
    FR_MKFS_ABORTED,
    FR_TIMEOUT,
    FR_LOCKED,
    FR_NOT_ENOUGH_CORE,
    FR_TOO_MANY_OPEN_FILES,
    FR_INVALID_PARAMETER
} FRESULT;

/* Dummy FATFS object */
typedef struct
{
    uint8_t dummy;
} FATFS;

/* Dummy FIL object */
typedef struct
{
    uint32_t fptr;
    uint32_t objsize;
} FIL;

/* File open mode flags */
#define FA_READ             0x01
#define FA_WRITE            0x02
#define FA_OPEN_EXISTING    0x00
#define FA_CREATE_NEW       0x04
#define FA_CREATE_ALWAYS    0x08
#define FA_OPEN_ALWAYS      0x10
#define FA_OPEN_APPEND      0x30

/* Function prototypes */
FRESULT f_mount(FATFS *fs, const char *path, BYTE opt);
FRESULT f_open(FIL *fp, const char *path, BYTE mode);
FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw);
FRESULT f_sync(FIL *fp);
FRESULT f_close(FIL *fp);
FRESULT f_lseek(FIL *fp, DWORD ofs);
DWORD   f_size(FIL *fp);

#endif /* FF_H */
