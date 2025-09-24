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
    OS_MESSAGE_NO_FLAGS = 0,
    OS_MESSAGE_FLAG_PERSISTENT = (1 << 0)
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

#define OSSendMessageAny(msgQueue_, msg_, flags_)	\
	OSSendMessage(msgQueue_, (OSMessage)(msg_), flags_)

#define OSReceiveMessageAny(msgQueue_, msgOut_, flags_)	\
	OSReceiveMessage(msgQueue_, (OSMessage *)(msgOut_), flags_)

#define OSJamMessageAny(msgQueue_, msg_, flags_)	\
	OSJamMessage(msgQueue_, (OSMessage)(msg_), flags_)

#ifdef __cplusplus
}
#endif

#endif
