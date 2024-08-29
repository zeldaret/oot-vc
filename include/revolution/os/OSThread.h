#ifndef _RVL_SDK_OS_THREAD_H
#define _RVL_SDK_OS_THREAD_H

#include "revolution/os/OSContext.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OS_PRIORITY_MIN 0
#define OS_PRIORITY_MAX 31

#define OS_THREAD_STACK_MAGIC 0xDEADBABE

typedef enum {
    OS_THREAD_STATE_EXITED = 0,
    OS_THREAD_STATE_READY = 1,
    OS_THREAD_STATE_RUNNING = 2,
    OS_THREAD_STATE_SLEEPING = 4,
    OS_THREAD_STATE_MORIBUND = 8
} OSThreadState;

typedef enum {
    OS_THREAD_DETACHED = (1 << 0)
} OSThreadFlag;

typedef struct OSThreadQueue {
    /* 0x0 */ struct OSThread* head;
    /* 0x4 */ struct OSThread* tail;
} OSThreadQueue;

typedef struct OSMutexQueue {
    /* 0x0 */ struct OSMutex* head;
    /* 0x4 */ struct OSMutex* tail;
} OSMutexQueue;

typedef struct OSThread {
    OSContext context;
    /* 0x2C8 */ u16 state;
    /* 0x2CA */ u16 flags;
    /* 0x2CC */ s32 suspend;
    /* 0x2D0 */ s32 priority;
    /* 0x2D4 */ s32 base;
    /* 0x2D8 */ u32 val;
    /* 0x2DC */ OSThreadQueue* queue;
    /* 0x2E0 */ struct OSThread* next;
    /* 0x2E4 */ struct OSThread* prev;
    /* 0x2E8 */ OSThreadQueue joinQueue;
    /* 0x2F0 */ struct OSMutex* mutex;
    /* 0x2F4 */ OSMutexQueue mutexQueue;
    /* 0x2FC */ struct OSThread* nextActive;
    /* 0x300 */ struct OSThread* prevActive;
    /* 0x304 */ u32* stackBegin;
    /* 0x308 */ u32* stackEnd;
    /* 0x30C */ s32 error;
    /* 0x310 */ void* specific[2];
} OSThread;

typedef void (*OSSwitchThreadCallback)(OSThread* currThread, OSThread* newThread);
typedef void* (*OSThreadFunc)(void* arg);

OSSwitchThreadCallback OSSetSwitchThreadCallback(OSSwitchThreadCallback callback);
void __OSThreadInit(void);
void OSSetCurrentThread(OSThread* thread);
void OSInitMutexQueue(OSMutexQueue* queue);
void OSInitThreadQueue(OSThreadQueue* queue);
OSThread* OSGetCurrentThread(void);
bool OSIsThreadSuspended(OSThread* thread);
bool OSIsThreadTerminated(OSThread* thread);
s32 OSDisableScheduler(void);
s32 OSEnableScheduler(void);
s32 __OSGetEffectivePriority(OSThread* thread);
void __OSPromoteThread(OSThread* thread, s32 prio);
void __OSReschedule(void);
void OSYieldThread(void);
bool OSCreateThread(OSThread* thread, OSThreadFunc func, void* funcArg, void* stackBegin, u32 stackSize, s32 prio,
                    u16 flags);
void OSExitThread(OSThread* thread);
void OSCancelThread(OSThread* thread);
bool OSJoinThread(OSThread* thread, void* val);
void OSDetachThread(OSThread* thread);
s32 OSResumeThread(OSThread* thread);
s32 OSSuspendThread(OSThread* thread);
void OSSleepThread(OSThreadQueue* queue);
void OSWakeupThread(OSThreadQueue* queue);
bool OSSetThreadPriority(OSThread* thread, s32 prio);
void OSClearStack(u8 val);
void OSSleepTicks(s64 ticks);

extern OSThread DefaultThread;

#ifdef __cplusplus
}
#endif

#endif
