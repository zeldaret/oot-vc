#ifndef _METROTRK_CIRCLE_BUFFER_H
#define _METROTRK_CIRCLE_BUFFER_H

#include "revolution/types.h"

typedef struct CircleBuffer {
    u8* unk0;
    u8* unk4;
    u8* unk8;
    u32 unkC;
    u32 mBytesToRead;
    u32 mBytesToWrite;
    unsigned int mSection;
} CircleBuffer;

u32 CBGetBytesAvailableForRead(CircleBuffer* cb);
u32 CBGetBytesAvailableForWrite(CircleBuffer* cb);
void CircleBufferInitialize(CircleBuffer* cb, u8* buf, u32 size);
void CircleBufferTerminate(CircleBuffer* cb);
int CircleBufferWriteBytes(CircleBuffer* cb, u8* buf, u32 size);
int CircleBufferReadBytes(CircleBuffer* cb, u8* buf, u32 size);

#endif
