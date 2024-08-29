#ifndef _RVL_SDK_OS_REBOOT_H
#define _RVL_SDK_OS_REBOOT_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void __OSReboot(u32 resetCode, u32 bootDol);
void OSGetSaveRegion(void** start, void** end);

#ifdef __cplusplus
}
#endif

#endif
