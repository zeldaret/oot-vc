#include "revolution/hbm/nw4hbm/snd/WaveSoundHandle.h"

namespace nw4hbm {
namespace snd {

void WaveSoundHandle::DetachSound() {
    if (IsAttachedSound()) {
        if (mSound->mTempSpecialHandle == this) {
            mSound->mTempSpecialHandle = nullptr;
        }
    }

    if (mSound != nullptr) {
        mSound = nullptr;
    }
}

} // namespace snd
} // namespace nw4hbm
