#ifndef _RVL_SDK_DVD_QUEUE_H
#define _RVL_SDK_DVD_QUEUE_H

#include "revolution/dvd/dvd.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DVD_PRIO_HIGHEST,
    DVD_PRIO_HIGH,
    DVD_PRIO_MEDIUM,
    DVD_PRIO_LOW,

    DVD_PRIO_MAX,
} DVDQueuePriority;

void __DVDClearWaitingQueue(void);
bool __DVDPushWaitingQueue(s32 prio, DVDCommandBlock* block);
DVDCommandBlock* __DVDPopWaitingQueue(void);
bool __DVDCheckWaitingQueue(void);
DVDCommandBlock* __DVDGetNextWaitingQueue(void);
bool __DVDDequeueWaitingQueue(const DVDCommandBlock* block);

#ifdef __cplusplus
}
#endif

#endif
