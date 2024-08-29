#ifndef _RVL_SDK_OS_EXEC_H
#define _RVL_SDK_OS_EXEC_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSExecParams {
    /* 0x0 */ int valid;
    /* 0x4 */ u32 restartCode;
    /* 0x8 */ u32 bootDol;
    /* 0xC */ void* regionStart;
    /* 0x10 */ void* regionEnd;
    /* 0x14 */ int argsUseDefault;
    /* 0x18 */ void* argsAddr;
} OSExecParams;

extern bool __OSInReboot;

void __OSGetExecParams(OSExecParams* out);
void __OSLaunchMenu(void);

#ifdef __cplusplus
}
#endif

#endif
