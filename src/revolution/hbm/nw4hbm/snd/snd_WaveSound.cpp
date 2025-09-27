#include "revolution/hbm/nw4hbm/snd/WaveSound.h"

#include "revolution/hbm/nw4hbm/snd/SoundInstanceManager.h"
#include "revolution/hbm/nw4hbm/snd/WaveSoundHandle.h"

namespace nw4hbm {
namespace snd {
namespace detail {

NW4R_UT_GET_DERIVED_RUNTIME_TYPEINFO(WaveSound, BasicSound);

WaveSound::WaveSound(SoundInstanceManager<WaveSound>* manager) : mManager(manager), mTempSpecialHandle(nullptr) {}

bool WaveSound::Prepare(const void* waveSoundBase, s32 waveSoundOffset, int voices,
                        const WsdTrack::WsdCallback* callback, u32 callbackData) {
    NW4HBMAssertPointerNonnull_Line(waveSoundBase, 67);
    NW4HBMAssertPointerNonnull_Line(callback, 68);
    NW4HBMAssertPointerNonnull_Line(GetSoundPlayer(), 69);
    InitParam();

    return mWsdPlayer.Prepare(waveSoundBase, waveSoundOffset, voices, callback, callbackData);
}

void WaveSound::Shutdown() {
    BasicSound::Shutdown();
    mManager->Free(this);
}

void WaveSound::SetChannelPriority(int priority) {
    NW4HBMAssertHeaderClampedLRValue_Line(priority, 0, 127, 133);
    mWsdPlayer.SetChannelPriority(priority);
}

void WaveSound::SetPlayerPriority(int priority) {
    BasicSound::SetPlayerPriority(priority);
    mManager->UpdatePriority(this, CalcCurrentPlayerPriority());
}

bool WaveSound::IsAttachedTempSpecialHandle() { return mTempSpecialHandle != NULL; }

void WaveSound::DetachTempSpecialHandle() { mTempSpecialHandle->DetachSound(); }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
