#ifndef NW4HBM_SND_WAVE_SOUND_HANDLE_H
#define NW4HBM_SND_WAVE_SOUND_HANDLE_H

#include "revolution/hbm/nw4hbm/snd/WaveSound.h"
#include "revolution/hbm/nw4hbm/snd/types.h"
#include "revolution/hbm/nw4hbm/ut.h"

namespace nw4hbm {
namespace snd {

class WaveSoundHandle : private ut::NonCopyable {
public:
    ~WaveSoundHandle() { DetachSound(); }

    void DetachSound();

    bool IsAttachedSound() const { return mSound != NULL; }

private:
    /* 0x00 */ detail::WaveSound* mSound;
};

} // namespace snd
} // namespace nw4hbm

#endif
