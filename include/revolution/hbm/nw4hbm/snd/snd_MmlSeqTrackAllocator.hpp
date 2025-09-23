#ifndef NW4R_SND_MML_SEQ_TRACK_ALLOCATOR_H
#define NW4R_SND_MML_SEQ_TRACK_ALLOCATOR_H

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // u32

#include "revolution/hbm/nw4hbm/snd/snd_InstancePool.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_SeqTrackAllocator.hpp"

/*******************************************************************************
 * classes and functions
 */

// forward declarations
namespace nw4hbm {
namespace snd {
namespace detail {
class MmlParser;
}
} // namespace snd
} // namespace nw4hbm
namespace nw4hbm {
namespace snd {
namespace detail {
class MmlSeqTrack;
}
} // namespace snd
} // namespace nw4hbm

namespace nw4hbm {
namespace snd {
namespace detail {
// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x314e0
class MmlSeqTrackAllocator : public SeqTrackAllocator {
    // methods
  public:
    // cdtors
    MmlSeqTrackAllocator(MmlParser* parser) : mParser(parser) {}

    // virtual function ordering
    // vtable SeqTrackAllocator
    virtual SeqTrack* AllocTrack(SeqPlayer* player);
    virtual void FreeTrack(SeqTrack* track);
    virtual int GetAllocatableTrackCount() const { return mTrackPool.Count(); }

    // methods
    u32 Create(void* buffer, u32 size);
    void Destroy(void* buffer, u32 size);

    // members
  private:
    /* base SeqTrackAllocator */ // size 0x04, offset 0x00
    MmlParser* mParser; // size 0x04, offset 0x04
    InstancePool<MmlSeqTrack> mTrackPool; // size 0x04, offset 0x08
}; // size 0x0c
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_MML_SEQ_TRACK_ALLOCATOR_H
