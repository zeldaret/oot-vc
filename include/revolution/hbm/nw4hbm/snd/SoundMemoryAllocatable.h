#ifndef NW4HBM_SND_SOUND_MEMORY_ALLOCATABLE_H
#define NW4HBM_SND_SOUND_MEMORY_ALLOCATABLE_H

#include "revolution/hbm/nw4hbm/snd/types.h"

namespace nw4hbm {
namespace snd {
        class SoundMemoryAllocatable {
            public:
                virtual ~SoundMemoryAllocatable() {}    // 0x08
                virtual void*   Alloc(u32 size) = 0;    // 0x0C
        };
    }
}

#endif // NW4HBM_SND_SOUND_MEMORY_ALLOCATABLE_H
