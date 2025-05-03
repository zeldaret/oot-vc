#ifndef _RVL_SDK_DSP_TASK_H
#define _RVL_SDK_DSP_TASK_H

#include "revolution/os/OSContext.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_SEND_MAIL_SYNC(x)           \
    do {                                \
        DSPSendMailToDSP((DSPMail)(x)); \
        while (DSPCheckMailToDSP()) {   \
            ;                           \
        }                               \
    } while (0)

// Forward declarations
typedef struct DSPTask;

typedef enum {
    DSP_TASK_ACTIVE = (1 << 0),
    DSP_TASK_CANCELED = (1 << 1),
} DSPTaskFlags;

typedef enum {
    DSP_TASK_STATE_0,
    DSP_TASK_STATE_1,
    DSP_TASK_STATE_2,
    DSP_TASK_STATE_3,
} DSPTaskState;

typedef void (*DSPTaskCallback)(struct DSPTask* task);

typedef struct DSPTask {
    /* 0x0 */ u32 state;
    /* 0x4 */ volatile u32 prio;
    /* 0x8 */ u32 flags;
    /* 0xC */ void* iramMmemAddr;
    /* 0x10 */ u32 iramMmemLen;
    /* 0x14 */ uintptr_t iramDspAddr;
    /* 0x18 */ void* dramMmemAddr;
    /* 0x1C */ u32 dramMmemLen;
    /* 0x20 */ uintptr_t dramDspAddr;
    /* 0x24 */ u16 startVector;
    /* 0x26 */ u16 resumeVector;
    /* 0x28 */ DSPTaskCallback initCallback;
    /* 0x2C */ DSPTaskCallback resumeCallback;
    /* 0x30 */ DSPTaskCallback doneCallback;
    /* 0x34 */ DSPTaskCallback requestCallback;
    /* 0x38 */ struct DSPTask* next;
    /* 0x3C */ struct DSPTask* prev;
    char UNK_0x40[0x50 - 0x40];
} DSPTask;

extern bool __DSP_rude_task_pending;
extern DSPTask* __DSP_rude_task;
extern DSPTask* __DSP_tmp_task;
extern DSPTask* __DSP_last_task;
extern DSPTask* __DSP_first_task;
extern DSPTask* __DSP_curr_task;

void __DSPHandler(__OSInterrupt intr, struct OSContext* ctx);
void __DSP_exec_task(DSPTask* task1, DSPTask* task2);
void __DSP_boot_task(DSPTask* task);
void __DSP_insert_task(DSPTask* task);
void __DSP_remove_task(DSPTask* task);

#ifdef __cplusplus
}
#endif

#endif
