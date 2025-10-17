#ifndef _RVL_SDK_MEM_HEAP_COMMON_H
#define _RVL_SDK_MEM_HEAP_COMMON_H

#include "mem_funcs.h"
#include "revolution/mem/mem_list.h"
#include "revolution/os.h"
#include "revolution/types.h"

#define MEM_HEAP_INVALID_HANDLE 0

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MEM_HEAP_OPT_CLEAR_ALLOC = (1 << 0),
    MEM_HEAP_OPT_DEBUG_FILL = (1 << 1),
    MEM_HEAP_OPT_CAN_LOCK = (1 << 2)
} MEMHeapOpt;

typedef struct MEMiHeapHead {
    /* 0x0 */ u32 magic;
    /* 0x4 */ MEMLink link;
    /* 0xC */ MEMList list;
    /* 0x18 */ u8* start;
    /* 0x1C */ u8* end;
    /* 0x20 */ OSMutex mutex;

    /* 0x38 */ union {
        u32 attribute;
        struct {
            u32 attribute_0_24 : 24;
            u32 opt : 8;
        };
    };
} MEMiHeapHead;

typedef MEMiHeapHead* MEMHeapHandle;

void MEMiInitHeapHead(MEMiHeapHead* heap, u32 magic, void* start, void* end, u16 opt);
void MEMiFinalizeHeap(MEMiHeapHead* heap);
MEMiHeapHead* MEMFindContainHeap(const void* memBlock);

static inline uintptr_t GetUIntPtr(const void* p) { return (uintptr_t)p; }

static inline void* AddU32ToPtr(const void* p, u32 ofs) { return (void*)(GetUIntPtr(p) + ofs); }

static inline void* SubU32ToPtr(const void* p, u32 ofs) { return (void*)(GetUIntPtr(p) - ofs); }

static inline const void* AddU32ToCPtr(const void* p, u32 ofs) { return (const void*)(GetUIntPtr(p) + ofs); }

static inline const void* SubU32ToCPtr(const void* p, u32 ofs) { return (const void*)(GetUIntPtr(p) - ofs); }

static inline s32 GetOffsetFromPtr(const void* start, const void* end) { return GetUIntPtr(end) - GetUIntPtr(start); }

static inline u16 GetOptForHeap(const MEMiHeapHead* heap) { return heap->opt; }

static inline void SetOptForHeap(MEMiHeapHead* heap, u16 opt) { heap->opt = (u8)opt; }

static inline void LockHeap(MEMiHeapHead* heap) {
    if (GetOptForHeap(heap) & MEM_HEAP_OPT_CAN_LOCK) {
        OSLockMutex(&heap->mutex);
    }
}

static inline void UnlockHeap(MEMiHeapHead* heap) {
    if (GetOptForHeap(heap) & MEM_HEAP_OPT_CAN_LOCK) {
        OSUnlockMutex(&heap->mutex);
    }
}

static inline void FillAllocMemory(MEMiHeapHead* heap, void* memBlock, u32 size) {
    if (GetOptForHeap(heap) & MEM_HEAP_OPT_CLEAR_ALLOC) {
        memset(memBlock, 0, size);
    }
}

static inline s32 MEMGetHeapTotalSize(MEMiHeapHead* heap) { return GetOffsetFromPtr(heap, heap->end); }

#ifdef __cplusplus
}
#endif

#endif
