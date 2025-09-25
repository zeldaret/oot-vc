#ifndef NW4R_SND_MML_SEQ_TRACK_ALLOCATOR_H
#define NW4R_SND_MML_SEQ_TRACK_ALLOCATOR_H

#include "revolution/types.h" // u32

#include "revolution/hbm/nw4hbm/snd/InstancePool.h"
#include "revolution/hbm/nw4hbm/snd/snd_SeqTrackAllocator.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

// Forward declarations
class MmlParser;
class MmlSeqTrack;
class SeqPlayer;
class SeqTrack;

class MmlSeqTrackAllocator : public SeqTrackAllocator {
  public:
    explicit MmlSeqTrackAllocator(MmlParser* parser) : mParser(parser) {}

    virtual SeqTrack* AllocTrack(SeqPlayer* player); // 0x0C
    virtual void FreeTrack(SeqTrack* track); // 0x10

    u32 Create(void* buffer, u32 size);
    void Destroy(void* buffer, u32 size);

  private:
    MmlParser* mParser; // 0x04
    InstancePool<MmlSeqTrack> mTrackPool; // 0x08
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_MML_SEQ_TRACK_ALLOCATOR_H
