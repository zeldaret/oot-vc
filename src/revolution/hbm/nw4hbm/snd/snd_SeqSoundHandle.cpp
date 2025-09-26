#include "revolution/hbm/nw4hbm/snd/SeqSoundHandle.h"

namespace nw4hbm {
    namespace snd {
        void SeqSoundHandle::DetachSound() {
            if (IsAttachedSound()) {
                if (mSound->mTempSpecialHandle == this) {
                    mSound->mTempSpecialHandle = nullptr;
                }
            }

            if (mSound != nullptr) {
                mSound = nullptr;
            }
        }
    }
}
