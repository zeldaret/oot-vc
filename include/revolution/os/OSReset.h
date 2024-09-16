#ifndef _RVL_SDK_OS_RESET_H
#define _RVL_SDK_OS_RESET_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OS_RESET_PRIO_GX 127

typedef bool (*OSShutdownFunction)(bool final, u32 event);

typedef enum {
    OS_SD_EVENT_SHUTDOWN = 2,
    OS_SD_EVENT_RESTART = 4,
    OS_SD_EVENT_RETURN_TO_MENU = 5,
    OS_SD_EVENT_LAUNCH_APP = 6,
} OSShutdownEvent;

typedef struct OSShutdownFunctionInfo {
    /* 0x0 */ OSShutdownFunction func;
    /* 0x4 */ u32 prio;
    /* 0x8 */ struct OSShutdownFunctionInfo* next;
    /* 0xC */ struct OSShutdownFunctionInfo* prev;
} OSShutdownFunctionInfo;

typedef struct OSShutdownFunctionQueue {
    /* 0x0 */ OSShutdownFunctionInfo* head;
    /* 0x4 */ OSShutdownFunctionInfo* tail;
} OSShutdownFunctionQueue;

void OSRegisterShutdownFunction(OSShutdownFunctionInfo* info);
bool __OSCallShutdownFunctions(u32 pass, u32 event);
void __OSShutdownDevices(u32 event);
void __OSGetDiscState(u8* out);
void OSShutdownSystem(void);
void OSRestart(u32 resetCode);
void OSReturnToMenu(void);
u32 OSGetResetCode(void);
void OSResetSystem(u32 arg0, u32 arg1, u32 arg2);

#ifdef __cplusplus
}
#endif

#endif
