#ifndef _DISK_H
#define _DISK_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Disk {
    /* 0x00 */ void* pHost;
} Disk; // size = 0x04

bool diskEvent(Disk* pDisk, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassDD;

#ifdef __cplusplus
}
#endif

#endif
