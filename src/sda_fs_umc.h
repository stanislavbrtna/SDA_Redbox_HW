#ifndef SDA_PLATFORM_FS_H
#define SDA_PLATFORM_FS_H

#include "FATFS/ff.h"
extern FATFS FatFs;

typedef struct {
  FIL fPointer;
  FRESULT fPointerRes;
} svp_file;

typedef struct {
  FRESULT res;
  DIR dir;
} svp_dir;

#endif
