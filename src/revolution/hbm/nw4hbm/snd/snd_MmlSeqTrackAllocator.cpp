#include "revolution/hbm/nw4hbm/snd/snd_MmlSeqTrackAllocator.hpp"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_MmlSeqTrackAllocator.cpp
 */

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/snd_InstancePool.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_MmlSeqTrack.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_SeqTrack.hpp"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {

SeqTrack* MmlSeqTrackAllocator::AllocTrack(SeqPlayer* player) {
    MmlSeqTrack* track = mTrackPool.Alloc();
    if (track) {
        track->SetSeqPlayer(player);
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
