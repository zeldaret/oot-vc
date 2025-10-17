#include "revolution/mem.h"
#include "revolution/os.h"

#define MEM_UNIT_HEAP_MAGIC 'UNTH'

// Size of base and unit heap head
#define MEM_UNIT_HEAP_HEAD_SIZE (sizeof(MEMiHeapHead) + sizeof(MEMiUnitHeapHead))

static MEMiUnitHeapHead* GetUnitHeapHeadPtrFromHeapHead_(const MEMiHeapHead* heap) {
    return AddU32ToPtr(heap, sizeof(MEMiHeapHead));
}

static MEMiHeapHead* GetHeapHeadPtrFromUnitHeapHead_(const MEMiUnitHeapHead* frm) {
    return SubU32ToPtr(frm, sizeof(MEMiHeapHead));
}

MEMiHeapHead* MEMCreateUnitHeapEx(void* heap, u32 heapSize, u32 memBlockSize, int align, u16 flags) {
    void* start = heap;
    void* end = AddU32ToPtr(heap, heapSize);
    MEMiUnitHeapHead* unit;
    void* memStart;
    u32 numBlocks;
    MEMiUnitHeapMBlock* mblock;
    int i;

    // Heap range
    start = ROUND_UP_PTR(start, 4);
    end = ROUND_DOWN_PTR(end, 4);

    // Ensure valid heap
    if (GetOffsetFromPtr(end, start) > 0) {
        return NULL;
    }

    unit = GetUnitHeapHeadPtrFromHeapHead_(start);

    memBlockSize = ROUND_UP(memBlockSize, align);
    memStart = ROUND_UP_PTR(AddU32ToPtr(unit, sizeof(MEMiUnitHeapHead)), align);

    if (GetOffsetFromPtr(end, memStart) > 0) {
        return NULL;
    }

    numBlocks = GetOffsetFromPtr(memStart, end) / memBlockSize;
    if (numBlocks == 0) {
        return NULL;
    }

    MEMiInitHeapHead(start, MEM_UNIT_HEAP_MAGIC, memStart, AddU32ToPtr(memStart, numBlocks * memBlockSize), flags);
    unit->freeList = memStart;
    unit->blockSize = memBlockSize;

    mblock = memStart;
    for (i = 0; i < numBlocks - 1; i++) {
        mblock->next = AddU32ToPtr(mblock, memBlockSize);
        mblock = mblock->next;
    }
    mblock->next = NULL;

    return start;
}

void* MEMAllocFromUnitHeap(MEMiHeapHead* heap) {
    MEMiUnitHeapHead* unit = GetUnitHeapHeadPtrFromHeapHead_(heap);
    MEMiUnitHeapMBlock* mblock;

    LockHeap(heap);
    mblock = unit->freeList;
    if (mblock != NULL) {
        unit->freeList = mblock->next;
    }
    UnlockHeap(heap);

    if (mblock != NULL) {
        FillAllocMemory(heap, mblock, unit->blockSize);
    }
    return mblock;
}

void MEMFreeToUnitHeap(MEMiHeapHead* heap, void* addr) {
    MEMiUnitHeapHead* unit = GetUnitHeapHeadPtrFromHeapHead_(heap);
    MEMiUnitHeapMBlock* mblock = addr;

    if (addr != NULL) {
        LockHeap(heap);
        mblock->next = unit->freeList;
        unit->freeList = mblock;
        UnlockHeap(heap);
    }
}

u32 MEMCountFreeBlockForUnitHeap(MEMiHeapHead* heap) {
    MEMiUnitHeapHead* unit = GetUnitHeapHeadPtrFromHeapHead_(heap);
    MEMiUnitHeapMBlock* mblock;
    u32 count = 0;

    LockHeap(heap);
    for (mblock = unit->freeList; mblock != NULL; mblock = mblock->next) {
        count++;
    }
    UnlockHeap(heap);

    return count;
}
