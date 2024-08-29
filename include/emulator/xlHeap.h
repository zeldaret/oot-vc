#ifndef _XL_HEAP_H
#define _XL_HEAP_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HEAP_COUNT 2

bool xlHeapCompact(s32 iHeap);
bool xlHeapTake(void** ppHeap, s32 nByteCount);
bool xlHeapFree(void** ppHeap);
bool xlHeapCopy(void* pHeapTarget, void* pHeapSource, s32 nByteCount);
bool xlHeapFill8(void* pHeap, s32 nByteCount, u32 nData);
bool xlHeapFill32(void* pHeap, s32 nByteCount, u32 nData);
bool xlHeapGetFree(s32* pnFreeBytes);
bool xlHeapSetup(void);
bool xlHeapReset(void);

extern s32 gnSizeHeap[HEAP_COUNT];
extern u32* gnHeapOS[HEAP_COUNT];

#ifdef __cplusplus
}
#endif

#endif
