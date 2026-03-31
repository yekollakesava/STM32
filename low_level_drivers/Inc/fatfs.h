#ifndef __FATFS_H
#define __FATFS_H

#include "ff.h"

extern FATFS SDFatFS;
extern char SDPath[4];

void MX_FATFS_Init(void);

#endif
