#include "revolution/hbm/nw4hbm/snd/snd_StrmSoundHandle.hpp"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_StrmSoundHandle.cpp
 */

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // nullptr

#include "revolution/hbm/nw4hbm/snd/snd_StrmSound.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace snd {

void StrmSoundHandle::DetachSound() {
    if (IsAttachedSound()) {
        if (mSound->mTempSpecialHandle == this) {
            mSound->mTempSpecialHandle = nullptr;
        }
    }

    if (mSound) {
        mSound = nullptr;
    }
}

} // namespace snd
} // namespace nw4hbm
