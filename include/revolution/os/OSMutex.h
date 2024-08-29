#ifndef _RVL_SDK_OS_MUTEX_H
#define _RVL_SDK_OS_MUTEX_H

#include "revolution/os/OSThread.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSMutex {
    /* 0x0 */ OSThreadQueue queue;
    /* 0x8 */ OSThread* thread;
    /* 0xC */ s32 lock;
    /* 0x10 */ struct OSMutex* next;
    /* 0x14 */ struct OSMutex* prev;
} OSMutex;

void OSInitMutex(OSMutex* mutex);
void OSLockMutex(OSMutex* mutex);
void OSUnlockMutex(OSMutex* mutex);
void __OSUnlockAllMutex(OSThread* thread);
bool OSTryLockMutex(OSMutex* mutex);

#ifdef __cplusplus
}
#endif

#endif
