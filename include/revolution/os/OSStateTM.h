#ifndef _RVL_SDK_OS_STATETM_H
#define _RVL_SDK_OS_STATETM_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*OSStateCallback)(void);

OSStateCallback OSSetResetCallback(OSStateCallback callback);
OSStateCallback OSSetPowerCallback(OSStateCallback callback);
bool __OSInitSTM(void);
void __OSShutdownToSBY(void);
void __OSHotReset(void);
bool __OSGetResetButtonStateRaw(void);
s32 __OSSetVIForceDimming(u32 arg0, u32 arg1, u32 arg2);
s32 __OSSetIdleLEDMode(u32 mode);
s32 __OSUnRegisterStateEvent(void);

#ifdef __cplusplus
}
#endif

#endif
