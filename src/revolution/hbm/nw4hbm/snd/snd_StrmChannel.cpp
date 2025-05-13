#include "revolution/hbm/snd.hpp"
#include "revolution/hbm/ut.hpp"

#include "cstring.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

void StrmBufferPool::Setup(void* pBase, u32 size, int count) {
    if (count == 0) {
        return;
    }

    ut::AutoInterruptLock lock;

    mBuffer = pBase;
    mBufferSize = size;

    mBlockSize = size / count;
    mBlockCount = count;

    mAllocCount = 0;
    std::memset(&mAllocFlags, 0, sizeof(mAllocFlags));
}

void StrmBufferPool::Shutdown() {
    ut::AutoInterruptLock lock;

    mBuffer = nullptr;
    mBufferSize = 0;

    mBlockSize = 0;
    mBlockCount = 0;
}

void* StrmBufferPool::Alloc() {
    ut::AutoInterruptLock lock;

    if (mAllocCount >= mBlockCount) {
        return nullptr;
    }

    int usableFlags = ut::RoundUp(mBlockCount, BITS_PER_BYTE) / BITS_PER_BYTE;

    for (int i = 0; i < usableFlags; i++) {
        u8 flag = static_cast<u8>(mAllocFlags[i]);

        // All blocks allocated in this flag set
        if (flag == 0xFF) {
            continue;
        }

        u8 mask = 1 << 0;

        for (int j = 0; j < 8; j++, mask <<= 1) {
            // Block represented by this bit is in use
            if (flag & mask) {
                continue;
            }

            mAllocFlags[i] |= mask;
            mAllocCount++;

            return ut::AddOffsetToPtr(mBuffer, mBlockSize * (j + i * BITS_PER_BYTE));
        }
    }

    return nullptr;
}

void StrmBufferPool::Free(void* pBuffer) {
    ut::AutoInterruptLock lock;

    s32 offset = ut::GetOffsetFromPtr(mBuffer, pBuffer);
    u32 block = offset / mBlockSize;

    u32 byte = block / BITS_PER_BYTE;
    u32 bit = block % BITS_PER_BYTE;

    mAllocFlags[byte] &= ~(1 << bit);
    mAllocCount--;
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
