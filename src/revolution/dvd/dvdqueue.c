#include "revolution/dvd.h"

typedef struct DVDWaitingQueue {
    struct DVDWaitingQueue* next; // at 0x0
    struct DVDWaitingQueue* prev; // at 0x4
} DVDWaitingQueue;

static DVDWaitingQueue WaitingQueue[DVD_PRIO_MAX];

void __DVDClearWaitingQueue(void) {
    u32 i;

    for (i = 0; i < DVD_PRIO_MAX; i++) {
        DVDCommandBlock* head = (DVDCommandBlock*)&WaitingQueue[i];
        head->next = head;
        head->prev = head;
    }
}

bool __DVDPushWaitingQueue(s32 prio, DVDCommandBlock* block) {
    bool enabled = OSDisableInterrupts();
    DVDCommandBlock* head = (DVDCommandBlock*)&WaitingQueue[prio];

    head->prev->next = block;
    block->prev = head->prev;
    block->next = head;
    head->prev = block;

    OSRestoreInterrupts(enabled);
    return true;
}

static DVDCommandBlock* PopWaitingQueuePrio(s32 prio) {
    bool enabled = OSDisableInterrupts();

    DVDCommandBlock* head = (DVDCommandBlock*)&WaitingQueue[prio];
    DVDCommandBlock* block = head->next;

    head->next = block->next;
    block->next->prev = head;

    OSRestoreInterrupts(enabled);

    block->next = NULL;
    block->prev = NULL;
    return block;
}

DVDCommandBlock* __DVDPopWaitingQueue(void) {
    u32 i;
    bool enabled = OSDisableInterrupts();

    for (i = 0; i < DVD_PRIO_MAX; i++) {
        DVDCommandBlock* head = (DVDCommandBlock*)&WaitingQueue[i];

        if (head->next != head) {
            OSRestoreInterrupts(enabled);
            return PopWaitingQueuePrio(i);
        }
    }

    OSRestoreInterrupts(enabled);
    return NULL;
}

bool __DVDCheckWaitingQueue(void) {
    u32 i;
    bool enabled = OSDisableInterrupts();

    for (i = 0; i < DVD_PRIO_MAX; i++) {
        DVDCommandBlock* head = (DVDCommandBlock*)&WaitingQueue[i];

        if (head->next != head) {
            OSRestoreInterrupts(enabled);
            return true;
        }
    }

    OSRestoreInterrupts(enabled);
    return false;
}

DVDCommandBlock* __DVDGetNextWaitingQueue(void) {
    u32 i;
    bool enabled = OSDisableInterrupts();

    for (i = 0; i < DVD_PRIO_MAX; i++) {
        DVDCommandBlock* head = (DVDCommandBlock*)&WaitingQueue[i];
        DVDCommandBlock* block = head->next;

        if (block != head) {
            OSRestoreInterrupts(enabled);
            return block;
        }
    }

    OSRestoreInterrupts(enabled);
    return NULL;
}

bool __DVDDequeueWaitingQueue(const DVDCommandBlock* block) {
    bool enabled = OSDisableInterrupts();

    DVDCommandBlock* prev = block->prev;
    DVDCommandBlock* next = block->next;

    if (prev == NULL || next == NULL) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    prev->next = next;
    next->prev = prev;

    OSRestoreInterrupts(enabled);
    return true;
}
