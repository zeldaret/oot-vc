#include "decomp.h"
#include "revolution/hbm/nw4hbm/snd.h"
#include "revolution/mem.h"

namespace nw4hbm {
namespace snd {
namespace detail {

FrameHeap::FrameHeap() :
    mHandle(nullptr) {}

FrameHeap::~FrameHeap() {
    if (IsValid()) {
        Destroy();
    }
}

bool FrameHeap::Create(void* startAddress, u32 size) {
    NW4HBMAssertPointerNonnull_Line(startAddress, 75);

    if (IsValid()) {
        Destroy();
    }

    void* endAddress = static_cast<u8*>(startAddress) + size;
    startAddress = ut::RoundUp(startAddress, 4);
    if (startAddress > endAddress) {
        return false;
    }

    mHandle = MEMCreateFrmHeap(startAddress, ut::GetOffsetFromPtr(startAddress, endAddress));
    if (mHandle == NULL) {
        return false;
    }

    if (!NewSection()) {
        return false;
    }

    return true;
}

void FrameHeap::Destroy() {
    if (!IsValid()) {
        return;
    }

    ClearSection();
    MEMFreeToFrmHeap(mHandle, MEM_FRM_HEAP_FREE_ALL);

    MEMDestroyFrmHeap(mHandle);
    mHandle = nullptr;
}

void FrameHeap::Clear() {
    NW4HBMAssert_Line(IsValid(), 135);
    ClearSection();
    MEMFreeToFrmHeap(mHandle, MEM_FRM_HEAP_FREE_ALL);
    NW4HBMAssertMessage_Line(NewSection(), 145, "FrameHeap::Clear(): NewSection is Failed");
}

void* FrameHeap::Alloc(u32 size, FreeCallback callback, void* callbackArg) {
    NW4HBMAssert_Line(IsValid(), 162);
    void* buffer = MEMAllocFromFrmHeapEx(mHandle, BLOCK_BUFFER_SIZE + ut::RoundUp(size, HEAP_ALIGN), HEAP_ALIGN);

    if (buffer == NULL) {
        return NULL;
    }

    Block* pBlock = new (buffer) Block(size, callback, callbackArg);
    mSectionList.GetBack().AppendBlock(pBlock);

    //! TODO: fake match?
    if (((u32)pBlock + 0x20) & 0x1F) {
        NW4HBMPanicMessage_Line(174, "FrameHeap::Alloc: Internal Error");
    }

    return pBlock->GetBufferAddr();
}

int FrameHeap::SaveState() {
    if (!MEMRecordStateForFrmHeap(mHandle, mSectionList.GetSize())) {
        return -1;
    }

    if (!NewSection()) {
        NW4HBMAssertMessage(MEMFreeByStateToFrmHeap(mHandle, 0),
                            "FrameHeap::SaveState(): MEMFreeByStateToFrmHeap is Failed");
        return -1;
    }

    return GetCurrentLevel();
}

DECOMP_FORCE("level is out of bounds(%d)\n%d <= level < %d not satisfied.");

void FrameHeap::LoadState(int id) {
    if (id == 0) {
        Clear();
        return;
    }

    while (id < static_cast<int>(mSectionList.GetSize())) {
        Section& rSection = mSectionList.GetBack();
        rSection.~Section();
        mSectionList.Erase(&rSection);
    }

    NW4HBMAssertMessage(MEMFreeByStateToFrmHeap(mHandle, id),
                        "FrameHeap::LoadState(): MEMFreeByStateToFrmHeap is Failed");
    NW4HBMAssertMessage(MEMRecordStateForFrmHeap(mHandle, mSectionList.GetSize()),
                        "FrameHeap::LoadState(): MEMRecordStateForFrmHeap is Failed");
    NW4HBMAssertMessage(NewSection(), "FrameHeap::LoadState(): NewSection is Failed");
}

int FrameHeap::GetCurrentLevel() const { return mSectionList.GetSize() - 1; }

u32 FrameHeap::GetFreeSize() const {
    NW4HBMAssert_Line(IsValid(), 295);

    u32 freeSize = MEMGetAllocatableSizeForFrmHeapEx(mHandle, HEAP_ALIGN);
    if (freeSize < BLOCK_BUFFER_SIZE) {
        return 0;
    }

    return ut::RoundDown(freeSize - BLOCK_BUFFER_SIZE, HEAP_ALIGN);
}

bool FrameHeap::NewSection() {
    void* pSection = MEMAllocFromFrmHeap(mHandle, sizeof(Section));
    if (pSection == NULL) {
        return false;
    }

    Section* newSection = new (pSection) Section();
    mSectionList.PushBack(newSection);
    return true;
}

void FrameHeap::ClearSection() {
    while (!mSectionList.IsEmpty()) {
        Section& rSection = mSectionList.GetBack();
        rSection.~Section();
        mSectionList.Erase(&rSection);
    }
}

FrameHeap::Section::~Section() {
    for (BlockList::Iterator it = mBlockList.GetEndIter(); it != mBlockList.GetBeginIter();) {
        (--it)->~Block();
    }
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
