#include "revolution/hbm/snd.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

SeqTrack* MmlSeqTrackAllocator::AllocTrack(SeqPlayer* pPlayer) {
    MmlSeqTrack* pTrack = mTrackPool.Alloc();

    if (pTrack != nullptr) {
        pTrack->SetSeqPlayer(pPlayer);
        pTrack->SetMmlParser(mParser);
    }

    return pTrack;
}

void MmlSeqTrackAllocator::FreeTrack(SeqTrack* pTrack) {
    pTrack->SetSeqPlayer(nullptr);
    mTrackPool.Free(static_cast<MmlSeqTrack*>(pTrack));
}

u32 MmlSeqTrackAllocator::Create(void* pBuffer, u32 size) { return mTrackPool.Create(pBuffer, size); }

void MmlSeqTrackAllocator::Destroy(void* pBuffer, u32 size) { mTrackPool.Destroy(pBuffer, size); }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
