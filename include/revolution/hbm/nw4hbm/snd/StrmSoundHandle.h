#ifndef NW4R_SND_STRM_SOUND_HANDLE_H
#define NW4R_SND_STRM_SOUND_HANDLE_H

#include "revolution/hbm/nw4hbm/snd/StrmSound.h"
#include "revolution/hbm/nw4hbm/ut/inlines.h"
#include "revolution/types.h"

namespace nw4hbm {
namespace snd {

class StrmSoundHandle : private ut::NonCopyable {
public:
    ~StrmSoundHandle() { DetachSound(); }

    void DetachSound();

    bool IsAttachedSound() const { return mSound != NULL; }

private:
    /* 0x00 */ detail::StrmSound* mSound;
};

} // namespace snd
} // namespace nw4hbm

#endif
