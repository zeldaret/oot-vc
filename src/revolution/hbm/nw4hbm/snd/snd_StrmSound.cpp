#include "revolution/hbm/nw4hbm/snd/StrmSound.h"
#include "revolution/hbm/nw4hbm/snd/StrmSoundHandle.h"

#include "revolution/hbm/nw4hbm/snd/SoundInstanceManager.h"

namespace nw4hbm {
namespace snd {
namespace detail {

NW4R_UT_GET_DERIVED_RUNTIME_TYPEINFO(StrmSound, BasicSound);

StrmSound::StrmSound(SoundInstanceManager<StrmSound>* manager) : mTempSpecialHandle(nullptr), mManager(manager) {}

bool StrmSound::Prepare(StrmBufferPool* bufferPool, StrmPlayer::StartOffsetType startOffsetType, s32 offset, int voices,
                        StrmPlayer::StrmCallback* callback, u32 callbackData) {
    NW4HBMAssertPointerNonnull_Line(GetSoundPlayer(), 66);
    NW4HBMAssertPointerNonnull_Line(bufferPool, 67);
    if (bufferPool == NULL) {
        return false;
    }

    InitParam();
    return mStrmPlayer.Prepare(bufferPool, startOffsetType, offset, voices, callback, callbackData);
}

void StrmSound::Shutdown() {
    BasicSound::Shutdown();
    mManager->Free(this);
}

void StrmSound::SetPlayerPriority(int priority) {
    BasicSound::SetPlayerPriority(priority);
    mManager->UpdatePriority(this, CalcCurrentPlayerPriority());
}

bool StrmSound::IsAttachedTempSpecialHandle() { return mTempSpecialHandle != NULL; }

void StrmSound::DetachTempSpecialHandle() { mTempSpecialHandle->DetachSound(); }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
