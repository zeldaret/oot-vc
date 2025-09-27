#ifndef REVOLUTION_MEM_UNIT_HEAP_H
#define REVOLUTION_MEM_UNIT_HEAP_H

#include "revolution/types.h"
#include "revolution/mem/mem_heapCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MEMiUntHeapMBlockHead MEMiUntHeapMBlockHead;
struct MEMiUntHeapMBlockHead {
    /* 0x00 */ MEMiUntHeapMBlockHead* next;
}; // size = 0x04

typedef struct MEMiUntHeapHead {
    /* 0x00 */ MEMiUntHeapMBlockHead* freeList;
    /* 0x04 */ u32 blockSize;
} MEMiUntHeapHead; // size = 0x08

MEMHeapHandle MEMCreateUnitHeapEx(void* start, u32 heapSize, u32 memBlockSize, int align, u16 flags);
void* MEMDestroyUnitHeap(MEMHeapHandle heap);

void* MEMAllocFromUnitHeap(MEMHeapHandle heap);
void MEMFreeToUnitHeap(MEMHeapHandle heap, void* addr);

u32 MEMCalcHeapSizeForUnitHeap(u32 unit_size, u32 unit_count, int align);

static u32 MEMGetMemBlockSizeForUnitHeap(MEMHeapHandle heap) {
    return ((MEMiUntHeapHead*)((u8*)heap + sizeof(MEMHeapHandle)))->blockSize;
}

static inline MEMHeapHandle MEMCreateUnitHeap(void* start, u32 heapSize, u32 memBlockSize) {
    return MEMCreateUnitHeapEx(start, heapSize, memBlockSize, 4, 0);
}

#ifdef __cplusplus
}
#endif

#endif
