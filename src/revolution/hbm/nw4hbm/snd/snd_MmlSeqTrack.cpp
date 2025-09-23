#include "revolution/hbm/nw4hbm/snd/snd_MmlSeqTrack.hpp"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_MmlSeqTrack.cpp
 */

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/snd/snd_MmlParser.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {

MmlSeqTrack::MmlSeqTrack() {}

MmlSeqTrack::ParseResult MmlSeqTrack::Parse(bool doNoteOn) { return mParser->Parse(this, doNoteOn); }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
