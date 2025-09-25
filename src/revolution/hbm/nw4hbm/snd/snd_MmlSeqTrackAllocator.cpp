#include "revolution/hbm/nw4hbm/snd/MmlSeqTrackAllocator.h"

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/InstancePool.h"
#include "revolution/hbm/nw4hbm/snd/MmlSeqTrack.h"
#include "revolution/hbm/nw4hbm/snd/SeqTrack.h"

#include "revolution/hbm/HBMAssert.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

SeqTrack* MmlSeqTrackAllocator::AllocTrack(SeqPlayer* seqPlayer) {
    MmlSeqTrack* track = mTrackPool.Alloc();

    if (track != NULL) {
        track->SetSeqPlayer(seqPlayer);
        track->SetMmlParser(mParser);
    }

    return track;
}

void MmlSeqTrackAllocator::FreeTrack(SeqTrack* track) {
    NW4HBMAssertPointerNonnull_Line(track, 59);
    track->SetSeqPlayer(nullptr);
    mTrackPool.Free(static_cast<MmlSeqTrack*>(track));
}

u32 MmlSeqTrackAllocator::Create(void* buffer, u32 size) { return mTrackPool.Create(buffer, size); }

void MmlSeqTrackAllocator::Destroy(void* buffer, u32 size) { mTrackPool.Destroy(buffer, size); }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
