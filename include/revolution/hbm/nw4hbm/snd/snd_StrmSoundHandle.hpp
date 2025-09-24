#ifndef NW4R_SND_STRM_SOUND_HANDLE_H
#define NW4R_SND_STRM_SOUND_HANDLE_H

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // nullptr

#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp" // ut::NonCopyable

/*******************************************************************************
 * types
 */

// forward declarations
namespace nw4hbm {
namespace snd {
namespace detail {
class StrmSound;
}
} // namespace snd
} // namespace nw4hbm

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace snd {
// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x3074e
class StrmSoundHandle : private ut::NonCopyable {
    // methods
  public:
    // methods
    bool IsAttachedSound() const { return mSound != nullptr; }

    void DetachSound();

    // members
  private:
    /* base NonCopyable */ // size 0x00, offset 0x00
    detail::StrmSound* mSound; // size 0x04, offset 0x00
}; // size 0x04
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_STRM_SOUND_HANDLE_H
