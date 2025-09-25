#include "revolution/hbm/nw4hbm/snd/WaveSound.h"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_WaveSound.cpp
 */

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/BasicSound.h"
#include "revolution/hbm/nw4hbm/snd/SoundInstanceManager.h"
#include "revolution/hbm/nw4hbm/snd/snd_WaveSoundHandle.hpp"
#include "revolution/hbm/nw4hbm/snd/WsdPlayer.h"

#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace snd {
namespace detail {
// .sbss
ut::detail::RuntimeTypeInfo const WaveSound::typeInfo(&BasicSound::typeInfo);
} // namespace detail
} // namespace snd
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {

WaveSound::WaveSound(SoundInstanceManager<WaveSound>* manager, int priority, int ambientPriority)
    : BasicSound(priority, ambientPriority), mTempSpecialHandle(nullptr), mManager(manager), mPreparedFlag(false) {}

bool WaveSound::Prepare(void const* waveSoundBase, s32 waveSoundOffset, WsdPlayer::StartOffsetType startOffsetType,
                        s32 offset, WsdPlayer::WsdCallback const* callback, register_t callbackData) {
    NW4HBMAssertPointerNonnull_Line(waveSoundBase, 74);
    NW4HBMAssertPointerNonnull_Line(callback, 75);

    InitParam();

    bool result = mWsdPlayer.Prepare(waveSoundBase, waveSoundOffset, startOffsetType, offset, GetVoiceOutCount(),
                                     callback, callbackData);
    if (!result) {
        return false;
    }

    mPreparedFlag = true;
    return true;
}

void WaveSound::Shutdown() {
    BasicSound::Shutdown();

    mManager->Free(this);
}

void WaveSound::SetChannelPriority(int priority) {
    // specifically not the source variant
    NW4HBMAssertHeaderClampedLRValue_Line(priority, BasicSound::PRIORITY_MIN, BasicSound::PRIORITY_MAX, 124);

    mWsdPlayer.SetChannelPriority(priority);
}

void WaveSound::SetReleasePriorityFix(bool flag) { mWsdPlayer.SetReleasePriorityFix(flag); }

void WaveSound::OnUpdatePlayerPriority() { mManager->UpdatePriority(this, CalcCurrentPlayerPriority()); }

bool WaveSound::IsAttachedTempSpecialHandle() { return mTempSpecialHandle != nullptr; }

void WaveSound::DetachTempSpecialHandle() { mTempSpecialHandle->DetachSound(); }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
