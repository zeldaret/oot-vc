#ifndef _RVL_SDK_MEM_ALLOCATOR_H
#define _RVL_SDK_MEM_ALLOCATOR_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct MEMAllocator;
typedef struct MEMiHeapHead;

typedef void* (*MEMAllocatorAllocFunc)(struct MEMAllocator* allocator, u32 size);
typedef void (*MEMAllocatorFreeFunc)(struct MEMAllocator* allocator, void* block);

typedef struct MEMAllocatorFuncs {
    /* 0x0 */ MEMAllocatorAllocFunc allocFunc;
    /* 0x4 */ MEMAllocatorFreeFunc freeFunc;
} MEMAllocatorFuncs;

typedef struct MEMAllocator {
    /* 0x0 */ const MEMAllocatorFuncs* funcs;
    /* 0x4 */ struct MEMiHeapHead* heap;
    /* 0x8 */ u32 heapParam1;
    /* 0xC */ u32 heapParam2;
} MEMAllocator;

void* MEMAllocFromAllocator(MEMAllocator* allocator, u32 size);
void MEMFreeToAllocator(MEMAllocator* allocator, void* block);

void MEMInitAllocatorForExpHeap(MEMAllocator* allocator, struct MEMiHeapHead* heap, s32 align);
void MEMInitAllocatorForFrmHeap(MEMAllocator* allocator, struct MEMiHeapHead* heap, s32 align);

#ifdef __cplusplus
}
#endif

#endif
