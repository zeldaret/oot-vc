#include "revolution/hbm/nw4hbm/snd/StrmSoundHandle.h"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_StrmSoundHandle.cpp
 */

#include "revolution/types.h" // nullptr

#include "revolution/hbm/nw4hbm/snd/StrmSound.h"

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
