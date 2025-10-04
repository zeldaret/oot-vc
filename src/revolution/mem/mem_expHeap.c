#include "revolution/mem.h"

#define MEM_EXP_HEAP_MAGIC 'EXPH'
#define MEM_EXP_HEAP_MBLOCK_FREE 'FR'
#define MEM_EXP_HEAP_MBLOCK_USED 'UD'

// Size of base and expandable heap head
#define MEM_EXP_HEAP_HEAD_SIZE (sizeof(MEMiHeapHead) + sizeof(MEMiExpHeapHead))

typedef struct Region {
    void* start;
    void* end;
} Region;

static void* AllocFromHead_(MEMiHeapHead* heap, u32 size, s32 align);
static void* AllocFromTail_(MEMiHeapHead* heap, u32 size, s32 align);

static MEMiExpHeapHead* GetExpHeapHeadPtrFromHeapHead_(const MEMiHeapHead* heap) {
    return AddU32ToPtr(heap, sizeof(MEMiHeapHead));
}

static MEMiHeapHead* GetHeapHeadPtrFromExpHeapHead_(const MEMiExpHeapHead* exp) {
    return SubU32ToPtr(exp, sizeof(MEMiHeapHead));
}

static void SetAllocMode_(MEMiExpHeapHead* exp, u16 mode) { exp->allocMode = mode; }

static u16 GetAllocMode_(const MEMiExpHeapHead* exp) { return exp->allocMode; }

static MEMiExpHeapMBlock* GetMBlockHeadPtr_(void* memBlock) { return SubU32ToPtr(memBlock, sizeof(MEMiExpHeapMBlock)); }

static const MEMiExpHeapMBlock* GetMBlockHeadCPtr_(const void* memBlock) {
    return SubU32ToCPtr(memBlock, sizeof(MEMiExpHeapMBlock));
}

static void* GetMemPtrForMBlock_(MEMiExpHeapMBlock* mblock) { return AddU32ToPtr(mblock, sizeof(MEMiExpHeapMBlock)); }

static void* GetMBlockEndAddr_(MEMiExpHeapMBlock* mblock) {
    return AddU32ToPtr(GetMemPtrForMBlock_(mblock), mblock->size);
}

static s32 GetAlignmentForMBlock_(const MEMiExpHeapMBlock* mblock) { return mblock->align; }

static MEMiExpHeapMBlock* InsertMBlock_(MEMiExpHeapMBlockList* list, MEMiExpHeapMBlock* prev,
                                        MEMiExpHeapMBlock* mblock) {
    MEMiExpHeapMBlock* next;

    // Fix prev link
    mblock->prev = prev;
    if (prev != NULL) {
        next = prev->next;
        prev->next = mblock;
    } else {
        next = list->head;
        list->head = mblock;
    }

    // Fix next link
    mblock->next = next;
    if (next != NULL) {
        next->prev = mblock;
    } else {
        list->tail = mblock;
    }

    return mblock;
}

static MEMiExpHeapMBlock* AppendMBlock_(MEMiExpHeapMBlockList* list, MEMiExpHeapMBlock* mblock) {
    return InsertMBlock_(list, list->tail, mblock);
}

static MEMiExpHeapMBlock* RemoveMBlock_(MEMiExpHeapMBlockList* list, MEMiExpHeapMBlock* mblock) {
    MEMiExpHeapMBlock* prev;
    MEMiExpHeapMBlock* next;

    prev = mblock->prev;
    next = mblock->next;

    // Fix prev link
    if (prev != NULL) {
        prev->next = next;
    } else {
        list->head = next;
    }

    // Fix next link
    if (next != NULL) {
        next->prev = prev;
    } else {
        list->tail = prev;
    }

    return prev;
}

static void GetRegionOfMBlock_(Region* region, MEMiExpHeapMBlock* mblock) {
    region->start = SubU32ToPtr(mblock, GetAlignmentForMBlock_(mblock));
    region->end = GetMBlockEndAddr_(mblock);
}

static MEMiExpHeapMBlock* InitMBlock_(Region* region, u16 state) {
    MEMiExpHeapMBlock* mblock;
    mblock = (MEMiExpHeapMBlock*)region->start;

    mblock->state = state;
    mblock->settings = 0;
    mblock->size = GetOffsetFromPtr(GetMemPtrForMBlock_(mblock), region->end);
    mblock->prev = NULL;
    mblock->next = NULL;

    return mblock;
}

static MEMiExpHeapMBlock* InitFreeMBlock_(Region* region) { return InitMBlock_(region, MEM_EXP_HEAP_MBLOCK_FREE); }

static MEMiHeapHead* InitExpHeap_(MEMiHeapHead* heap, void* end, u16 opt) {
    MEMiExpHeapMBlock* mblock;
    MEMiExpHeapHead* exp;
    Region region;

    exp = GetExpHeapHeadPtrFromHeapHead_(heap);
    MEMiInitHeapHead(heap, MEM_EXP_HEAP_MAGIC, AddU32ToPtr(heap, MEM_EXP_HEAP_HEAD_SIZE), end, opt);
    exp->group = 0;
    exp->SHORT_0x12 = 0;
    SetAllocMode_(exp, MEM_EXP_HEAP_ALLOC_FAST);

    region.start = heap->start;
    region.end = heap->end;

    mblock = InitFreeMBlock_(&region);
    exp->freeMBlocks.head = mblock;
    exp->freeMBlocks.tail = mblock;

    exp->usedMBlocks.head = NULL;
    exp->usedMBlocks.tail = NULL;

    return heap;
}

static void* AllocUsedBlockFromFreeBlock_(MEMiExpHeapHead* exp, MEMiExpHeapMBlock* mblock, void* memPtr, u32 size,
                                          u16 allocDir) {
    Region region;
    void* allocStart;
    void* allocEnd;
    void* allocHead;
    void* mblockStart;
    void* mblockEnd;
    MEMiExpHeapMBlock* prev;

    allocEnd = AddU32ToPtr(memPtr, size);
    allocStart = allocHead = GetMBlockHeadPtr_(memPtr);

    GetRegionOfMBlock_(&region, mblock);
    mblockStart = region.start;
    mblockEnd = region.end;

    prev = mblock->prev;
    RemoveMBlock_(&exp->freeMBlocks, mblock);

    if (GetOffsetFromPtr(mblockStart, allocStart) < sizeof(MEMiExpHeapMBlock) + 4) {
        allocStart = mblockStart;
    } else {
        region.start = mblockStart;
        region.end = allocStart;
        prev = InsertMBlock_(&exp->freeMBlocks, prev, InitFreeMBlock_(&region));
    }

    if (GetOffsetFromPtr(allocEnd, mblockEnd) < sizeof(MEMiExpHeapMBlock) + 4) {
        allocEnd = mblockEnd;
    } else {
        region.start = allocEnd;
        region.end = mblockEnd;
        prev = InsertMBlock_(&exp->freeMBlocks, prev, InitFreeMBlock_(&region));
    }

    FillAllocMemory(GetHeapHeadPtrFromExpHeapHead_(exp), allocStart, GetOffsetFromPtr(allocStart, allocEnd));

    region.start = allocHead;
    region.end = allocEnd;
    mblock = InitMBlock_(&region, MEM_EXP_HEAP_MBLOCK_USED);
    mblock->allocDir = allocDir;
    mblock->align = GetOffsetFromPtr(allocStart, allocHead);
    mblock->group = exp->group;
    AppendMBlock_(&exp->usedMBlocks, mblock);

    return memPtr;
}

static void* AllocFromHead_(MEMiHeapHead* heap, u32 size, s32 align) {
    MEMiExpHeapHead* exp;
    bool allocFast;
    MEMiExpHeapMBlock* mblock;
    MEMiExpHeapMBlock* bestBlock;
    u32 bestSize;
    void* bestAddr;

    exp = GetExpHeapHeadPtrFromHeapHead_(heap);
    allocFast = GetAllocMode_(exp) == MEM_EXP_HEAP_ALLOC_FAST;
    bestBlock = NULL;
    bestSize = -1;
    bestAddr = NULL;

    for (mblock = exp->freeMBlocks.head; mblock != NULL; mblock = mblock->next) {
        void* const memPtr = GetMemPtrForMBlock_(mblock);
        void* const addr = ROUND_UP_PTR(memPtr, align);

        if (mblock->size >= GetOffsetFromPtr(memPtr, addr) + size && bestSize > mblock->size) {
            bestBlock = mblock;
            bestSize = mblock->size;
            bestAddr = addr;
            if (allocFast || mblock->size == size) {
                break;
            }
        }
    }

    if (bestBlock != NULL) {
        return AllocUsedBlockFromFreeBlock_(exp, bestBlock, bestAddr, size, 0);
    } else {
        return NULL;
    }
}

void* AllocFromTail_(MEMiHeapHead* heap, u32 size, s32 align) {
    MEMiExpHeapHead* exp;
    bool allocFast;
    MEMiExpHeapMBlock* mblock;
    MEMiExpHeapMBlock* bestBlock;
    u32 bestSize;
    void* bestAddr;

    exp = GetExpHeapHeadPtrFromHeapHead_(heap);
    allocFast = GetAllocMode_(exp) == MEM_EXP_HEAP_ALLOC_FAST;
    bestBlock = NULL;
    bestSize = -1;
    bestAddr = NULL;

    for (mblock = exp->freeMBlocks.tail; mblock != NULL; mblock = mblock->prev) {
        void* const memPtr = GetMemPtrForMBlock_(mblock);
        void* const addr = ROUND_DOWN_PTR(SubU32ToPtr(AddU32ToPtr(memPtr, mblock->size), size), align);

        if (GetOffsetFromPtr(memPtr, addr) >= 0 && bestSize > mblock->size) {
            bestBlock = mblock;
            bestSize = mblock->size;
            bestAddr = addr;
            if (allocFast || mblock->size == size) {
                break;
            }
        }
    }

    if (bestBlock != NULL) {
        return AllocUsedBlockFromFreeBlock_(exp, bestBlock, bestAddr, size, 1);
    } else {
        return NULL;
    }
}

bool RecycleRegion_(MEMiExpHeapHead* exp, Region* region) {
    Region newRegion;
    MEMiExpHeapMBlock* mblock;
    MEMiExpHeapMBlock* prev;

    prev = NULL;
    newRegion = *region;

    for (mblock = exp->freeMBlocks.head; mblock != NULL; mblock = mblock->next) {
        if (mblock < region->start) {
            prev = mblock;
        } else {
            if (mblock == region->end) {
                newRegion.end = GetMBlockEndAddr_(mblock);
                RemoveMBlock_(&exp->freeMBlocks, mblock);
            }
            break;
        }
    }

    if (prev != NULL && GetMBlockEndAddr_(prev) == region->start) {
        newRegion.start = prev;
        prev = RemoveMBlock_(&exp->freeMBlocks, prev);
    }

    if (GetOffsetFromPtr(newRegion.start, newRegion.end) < sizeof(MEMiExpHeapMBlock)) {
        return false;
    }

    InsertMBlock_(&exp->freeMBlocks, prev, InitFreeMBlock_(&newRegion));
    return true;
}

MEMiHeapHead* MEMCreateExpHeapEx(void* start, u32 size, u16 opt) {
    void* end = AddU32ToPtr(start, size);

    // Heap range
    end = ROUND_DOWN_PTR(end, 4);
    start = ROUND_UP_PTR(start, 4);

    // Ensure valid heap
    if (GetUIntPtr(start) > GetUIntPtr(end) || GetOffsetFromPtr(start, end) < MEM_EXP_HEAP_MIN_SIZE) {
        return NULL;
    }

    return InitExpHeap_(start, end, opt);
}

MEMiHeapHead* MEMDestroyExpHeap(MEMiHeapHead* heap) {
    MEMiFinalizeHeap(heap);
    return heap;
}

void* MEMAllocFromExpHeapEx(MEMiHeapHead* heap, u32 size, s32 Align) {
    void* memBlock;

    if (size == 0) {
        size = 1;
    }
    size = ROUND_UP(size, 4);

    LockHeap(heap);

    // Alignment sign determines alloc direction
    if (Align >= 0) {
        memBlock = AllocFromHead_(heap, size, Align);
    } else {
        memBlock = AllocFromTail_(heap, size, -Align);
    }

    UnlockHeap(heap);

    return memBlock;
}

void MEMFreeToExpHeap(MEMiHeapHead* heap, void* memBlock) {
    MEMiExpHeapMBlock* mblock;
    MEMiExpHeapHead* exp;
    Region region;

    if (memBlock == NULL) {
        return;
    }

    exp = GetExpHeapHeadPtrFromHeapHead_(heap);
    mblock = GetMBlockHeadPtr_(memBlock);

    LockHeap(heap);

    GetRegionOfMBlock_(&region, mblock);
    RemoveMBlock_(&exp->usedMBlocks, mblock);
    RecycleRegion_(exp, &region);

    UnlockHeap(heap);
}
