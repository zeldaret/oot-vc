#include "metrotrk/CircleBuffer.h"
#include "metrotrk/MWCriticalSection_gc.h"
#include "metrotrk/trk.h"

u32 CBGetBytesAvailableForRead(CircleBuffer* cb) { return cb->mBytesToRead; }

void CircleBufferInitialize(CircleBuffer* cb, u8* buf, u32 size) {
    cb->unk8 = buf;
    cb->unkC = size;
    cb->unk0 = cb->unk8;
    cb->unk4 = cb->unk8;
    cb->mBytesToRead = 0;
    cb->mBytesToWrite = cb->unkC;
    MWInitializeCriticalSection(&(cb->mSection));
}

int CircleBufferWriteBytes(CircleBuffer* cb, u8* buf, u32 size) {
    u32 r29;

    if (size > cb->mBytesToWrite) {
        return -1;
    }

    MWEnterCriticalSection(&(cb->mSection));

    r29 = cb->unkC - (cb->unk4 - cb->unk8);

    if (r29 >= size) {
        memcpy(cb->unk4, buf, size);
        cb->unk4 += size;
    } else {
        memcpy(cb->unk4, buf, r29);
        memcpy(cb->unk8, buf + r29, size - r29);
        cb->unk4 = cb->unk8 + size - r29;
    }

    if (cb->unkC == cb->unk4 - cb->unk8) {
        cb->unk4 = cb->unk8;
    }

    cb->mBytesToWrite -= size;
    cb->mBytesToRead += size;

    MWExitCriticalSection(&(cb->mSection));

    return 0;
}

int CircleBufferReadBytes(CircleBuffer* cb, u8* buf, u32 size) {
    u32 r29;

    if (size > cb->mBytesToRead) {
        return -1;
    }

    MWEnterCriticalSection(&(cb->mSection));

    r29 = cb->unkC - (cb->unk0 - cb->unk8);

    if (size < r29) {
        memcpy(buf, cb->unk0, size);
        cb->unk0 += size;
    } else {
        memcpy(buf, cb->unk0, r29);
        memcpy(buf + r29, cb->unk8, size - r29);
        cb->unk0 = cb->unk8 + size - r29;
    }

    if (cb->unkC == cb->unk0 - cb->unk8) {
        cb->unk0 = cb->unk8;
    }

    cb->mBytesToWrite += size;
    cb->mBytesToRead -= size;

    MWExitCriticalSection(&(cb->mSection));

    return 0;
}
