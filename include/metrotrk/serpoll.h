#ifndef _TRK_SERPOLL_H
#define _TRK_SERPOLL_H

#include "metrotrk/msgbuf.h"
#include "metrotrk/trk.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char FCSType;

typedef enum ReceiverState {
    kWaitFlag = 0,
    kFoundFlag = 1,
    kInFrame = 2,
    kFrameOverflow = 3
} ReceiverState;

typedef struct FramingState {
    /* 0x00 */ MessageBufferID fBufferID;
    /* 0x00 */ MessageBuffer* fBuffer;
    /* 0x00 */ ReceiverState fReceiveState;
    /* 0x00 */ int fEscape;
    /* 0x00 */ FCSType fFCS;
} FramingState;

void TRKGetInput(void);
void TRKProcessInput(int);
DSError TRKInitializeSerialHandler(void);
DSError TRKTerminateSerialHandler(void);

extern void* gTRKInputPendingPtr;

#ifdef __cplusplus
}
#endif

#endif
