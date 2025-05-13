#ifndef NW4R_SND_SOUND_MEMORY_ALLOCATABLE_H
#define NW4R_SND_SOUND_MEMORY_ALLOCATABLE_H
#include "revolution/types.h"

namespace nw4hbm {
namespace snd {

class SoundMemoryAllocatable {
  public:
    virtual ~SoundMemoryAllocatable() {} // at 0x8
    virtual void* Alloc(u32 size) = 0; // at 0xC
};

} // namespace snd
} // namespace nw4hbm

#endif
