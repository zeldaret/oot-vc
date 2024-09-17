#ifndef _RVL_SDK_MEM_EXP_HEAP_H
#define _RVL_SDK_MEM_EXP_HEAP_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_EXP_HEAP_MIN_SIZE (sizeof(MEMiHeapHead) + sizeof(MEMiExpHeapHead) + sizeof(MEMiExpHeapMBlock) + 4)

// Forward declarations
typedef struct MEMiHeapHead;

typedef enum {
    MEM_EXP_HEAP_ALLOC_FAST, //!< When allocating memory blocks, take the first
                             //!< usable found block rather than trying to
                             //!< find a more optimal block
} MEMiExpHeapAllocMode;

typedef struct MEMiExpHeapMBlock {
    /* 0x0 */ u16 state;
    /* 0x2 */ union {
        u16 settings;
        struct {
            u16 allocDir : 1;
            u16 align : 7;
            u16 group : 8;
        };
    };
    /* 0x4 */ u32 size;
    /* 0x8 */ struct MEMiExpHeapMBlock* prev;
    /* 0xC */ struct MEMiExpHeapMBlock* next;
} MEMiExpHeapMBlock;

typedef struct MEMiExpHeapMBlockList {
    /* 0x0 */ MEMiExpHeapMBlock* head;
    /* 0x4 */ MEMiExpHeapMBlock* tail;
} MEMiExpHeapMBlockList;

// Placed in heap after base heap head
typedef struct MEMiExpHeapHead {
    /* 0x0 */ MEMiExpHeapMBlockList freeMBlocks;
    /* 0x8 */ MEMiExpHeapMBlockList usedMBlocks;
    /* 0x10 */ u16 group;
    /* 0x12 */ union {
        u16 SHORT_0x12;
        struct {
            u16 SHORT_0x12_0_15 : 15;
            u16 allocMode : 1;
        };
    };
} MEMiExpHeapHead;

struct MEMiHeapHead* MEMCreateExpHeapEx(void* start, u32 size, u16 opt);
struct MEMiHeapHead* MEMDestroyExpHeap(struct MEMiHeapHead* heap);
void* MEMAllocFromExpHeapEx(struct MEMiHeapHead* heap, u32 size, s32 align);
u32 MEMResizeForMBlockExpHeap(struct MEMiHeapHead* heap, void* memBlock, u32 size);
void MEMFreeToExpHeap(struct MEMiHeapHead* heap, void* memBlock);
u32 MEMAdjustExpHeap(struct MEMiHeapHead* heap);

static inline struct MEMiHeapHead* MEMCreateExpHeap(void* start, u32 size) {
    return MEMCreateExpHeapEx(start, size, 0);
}

static inline void* MEMAllocFromExpHeap(struct MEMiHeapHead* heap, u32 size) {
    return MEMAllocFromExpHeapEx(heap, size, 4);
}

#ifdef __cplusplus
}
#endif

#endif
