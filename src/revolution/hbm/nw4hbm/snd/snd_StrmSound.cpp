#include "revolution/hbm/snd.hpp"
#include "revolution/hbm/ut.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

NW4R_UT_RTTI_DEF_DERIVED(StrmSound, BasicSound);

StrmSound::StrmSound(SoundInstanceManager<StrmSound>* pManager) : mManager(pManager), mTempSpecialHandle(nullptr) {}

bool StrmSound::Prepare(StrmBufferPool* pPool, StrmPlayer::StartOffsetType offsetType, s32 offset, int voices,
                        ut::FileStream* pStream) {
    if (pPool == nullptr) {
        return false;
    }

    InitParam();

    if (!mStrmPlayer.Setup(pPool)) {
        return false;
    }

    if (!mStrmPlayer.Prepare(pStream, voices, offsetType, offset)) {
        mStrmPlayer.Shutdown();
        return false;
    }

    return true;
}

void StrmSound::Shutdown() {
    BasicSound::Shutdown();
    mManager->Free(this);
}

void StrmSound::SetPlayerPriority(int priority) {
    BasicSound::SetPlayerPriority(priority);
    mManager->UpdatePriority(this, CalcCurrentPlayerPriority());
}

bool StrmSound::IsAttachedTempSpecialHandle() { return mTempSpecialHandle != nullptr; }

void StrmSound::DetachTempSpecialHandle() { mTempSpecialHandle->DetachSound(); }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
