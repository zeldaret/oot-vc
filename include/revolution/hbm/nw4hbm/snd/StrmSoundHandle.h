#ifndef NW4R_SND_STRM_SOUND_HANDLE_H
#define NW4R_SND_STRM_SOUND_HANDLE_H

#include "revolution/hbm/nw4hbm/snd/StrmSound.h"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp"
#include "revolution/types.h"

namespace nw4hbm {
namespace snd {

class StrmSoundHandle : private ut::NonCopyable {
  public:
    ~StrmSoundHandle() { DetachSound(); }

    void DetachSound();

    bool IsAttachedSound() const { return mSound != NULL; }

  private:
    detail::StrmSound* mSound; // 0x00
};

} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_STRM_SOUND_HANDLE_H
