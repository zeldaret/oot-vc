#ifndef _RVL_SDK_OS_MESSAGE_H
#define _RVL_SDK_OS_MESSAGE_H

#include "revolution/os/OSThread.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// General-purpose typedef
typedef void* OSMessage;

typedef enum {
    OS_MSG_PERSISTENT = (1 << 0)
} OSMessageFlags;

typedef struct OSMessageQueue {
    /* 0x0 */ OSThreadQueue sendQueue;
    /* 0x8 */ OSThreadQueue recvQueue;
    /* 0x10 */ OSMessage* buffer;
    /* 0x14 */ s32 capacity;
    /* 0x18 */ s32 front;
    /* 0x1C */ s32 size;
} OSMessageQueue;

void OSInitMessageQueue(OSMessageQueue* queue, OSMessage* buffer, s32 capacity);
bool OSSendMessage(OSMessageQueue* queue, OSMessage mesg, u32 flags);
bool OSReceiveMessage(OSMessageQueue* queue, OSMessage* mesg, u32 flags);
bool OSJamMessage(OSMessageQueue* queue, OSMessage mesg, u32 flags);

#ifdef __cplusplus
}
#endif

#endif
