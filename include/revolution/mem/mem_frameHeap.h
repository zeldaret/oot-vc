#ifndef _RVL_SDK_MEM_FRAME_HEAP_H
#define _RVL_SDK_MEM_FRAME_HEAP_H

#include "revolution/mem/mem_heapCommon.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_FRM_HEAP_MIN_SIZE (sizeof(MEMiHeapHead) + sizeof(MEMiFrmHeapHead))

// Forward declarations
typedef struct MEMiHeapHead;

// Specify how to free memory
typedef enum {
    MEM_FRM_HEAP_FREE_TO_HEAD = (1 << 0),
    MEM_FRM_HEAP_FREE_TO_TAIL = (1 << 1),
    MEM_FRM_HEAP_FREE_ALL = MEM_FRM_HEAP_FREE_TO_HEAD | MEM_FRM_HEAP_FREE_TO_TAIL
} MEMiFrmFreeFlag;

typedef struct MEMiFrmHeapState {
    /* 0x0 */ u32 id;
    /* 0x4 */ u8* head;
    /* 0x8 */ u8* tail;
    /* 0xC */ struct MEMiFrmHeapState* next;
} MEMiFrmHeapState;

// Placed in heap after base heap head
typedef struct MEMiFrmHeapHead {
    /* 0x0 */ u8* head;
    /* 0x4 */ u8* tail;
    /* 0x8 */ MEMiFrmHeapState* states;
} MEMiFrmHeapHead;

struct MEMiHeapHead* MEMCreateFrmHeapEx(void* start, u32 size, u16 opt);
struct MEMiHeapHead* MEMDestroyFrmHeap(struct MEMiHeapHead* heap);
void* MEMAllocFromFrmHeapEx(struct MEMiHeapHead* heap, u32 size, s32 align);
void MEMFreeToFrmHeap(struct MEMiHeapHead* heap, u32 flags);
u32 MEMGetAllocatableSizeForFrmHeapEx(struct MEMiHeapHead* heap, s32 align);
bool MEMRecordStateForFrmHeap(struct MEMiHeapHead* heap, u32 id);
bool MEMFreeByStateToFrmHeap(struct MEMiHeapHead* heap, u32 id);
u32 MEMResizeForMBlockFrmHeap(struct MEMiHeapHead* heap, void* memBlock, u32 size);

static inline struct MEMiHeapHead* MEMCreateFrmHeap(void* start, u32 size) {
    return MEMCreateFrmHeapEx(start, size, 0);
}

static inline void* MEMAllocFromFrmHeap(struct MEMiHeapHead* heap, u32 size) {
    return MEMAllocFromFrmHeapEx(heap, size, 4);
}

static inline u32 MEMGetAllocatableSizeForFrmHeap(struct MEMiHeapHead* heap) {
    return MEMGetAllocatableSizeForFrmHeapEx(heap, 4);
}

#ifdef __cplusplus
}
#endif

#endif
