#ifndef NW4R_SND_SEQ_SOUND_HANDLE_H
#define NW4R_SND_SEQ_SOUND_HANDLE_H

#include "revolution/hbm/nw4hbm/snd/types.h"

#include "revolution/hbm/nw4hbm/snd/BasicSound.h"
#include "revolution/hbm/nw4hbm/snd/SeqPlayer.h"
#include "revolution/hbm/nw4hbm/snd/SeqSound.h"

#include "revolution/hbm/ut.hpp"

namespace nw4hbm {
namespace snd {
class SeqSoundHandle : private ut::NonCopyable {
  public:
    ~SeqSoundHandle() { DetachSound(); }

    void DetachSound();

    bool IsAttachedSound() const { return mSound != NULL; }

  private:
    detail::SeqSound* mSound; // 0x00
};

} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_SEQ_SOUND_HANDLE_H