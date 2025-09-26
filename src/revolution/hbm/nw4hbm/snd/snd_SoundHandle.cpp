#include "revolution/hbm/nw4hbm/snd/SoundHandle.h"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_SoundHandle.cpp
 */

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // nullptr

#include "revolution/hbm/nw4hbm/snd/BasicSound.h"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace snd {

void SoundHandle::detail_AttachSound(detail::BasicSound* sound) {
    NW4HBMAssertPointerNonnull_Line(sound, 85);

    mSound = sound;

    if (sound->IsAttachedGeneralHandle()) {
        mSound->DetachGeneralHandle();
    }

    mSound->mGeneralHandle = this;
}

void SoundHandle::DetachSound() {
    if (IsAttachedSound()) {
        if (mSound->mGeneralHandle == this) {
            mSound->mGeneralHandle = nullptr;
        }

        if (mSound->mTempGeneralHandle == this) {
            mSound->mTempGeneralHandle = nullptr;
        }
    }

    if (mSound) {
        mSound = nullptr;
    }
}

} // namespace snd
} // namespace nw4hbm
