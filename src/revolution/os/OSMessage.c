#include "revolution/os.h"

void OSInitMessageQueue(OSMessageQueue* queue, OSMessage* buffer, s32 capacity) {
    OSInitThreadQueue(&queue->sendQueue);
    OSInitThreadQueue(&queue->recvQueue);
    queue->buffer = buffer;
    queue->capacity = capacity;
    queue->front = 0;
    queue->size = 0;
}

bool OSSendMessage(OSMessageQueue* queue, OSMessage mesg, u32 flags) {
    s32 mesgId;
    bool enabled = OSDisableInterrupts();

    while (queue->capacity <= queue->size) {
        if (!(flags & OS_MESSAGE_FLAG_PERSISTENT)) {
            OSRestoreInterrupts(enabled);
            return false;
        }

        OSSleepThread(&queue->sendQueue);
    }

    mesgId = (queue->front + queue->size) % queue->capacity;
    queue->buffer[mesgId] = mesg;
    queue->size++;

    OSWakeupThread(&queue->recvQueue);
    OSRestoreInterrupts(enabled);
    return true;
}

bool OSReceiveMessage(OSMessageQueue* queue, OSMessage* mesg, u32 flags) {
    bool enabled = OSDisableInterrupts();

    while (queue->size == 0) {
        if (!(flags & OS_MESSAGE_FLAG_PERSISTENT)) {
            OSRestoreInterrupts(enabled);
            return false;
        }

        OSSleepThread(&queue->recvQueue);
    }

    if (mesg != NULL) {
        *mesg = queue->buffer[queue->front];
    }
    queue->front++;
    queue->front %= queue->capacity;
    queue->size--;

    OSWakeupThread(&queue->sendQueue);
    OSRestoreInterrupts(enabled);
    return true;
}
