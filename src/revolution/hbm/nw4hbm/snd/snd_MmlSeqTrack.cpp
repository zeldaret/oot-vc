#include "revolution/hbm/nw4hbm/snd/MmlSeqTrack.h"
#include "revolution/hbm/nw4hbm/snd/MmlParser.h"

namespace nw4hbm {
namespace snd {
namespace detail {

MmlSeqTrack::MmlSeqTrack() {
    mMmlParserParam.noteWaitFlag = true;
    mMmlParserParam.tieFlag = false;
    mMmlParserParam.cmpFlag = true;
    mMmlParserParam.callStackDepth = 0;
}

ParseResult MmlSeqTrack::Parse(bool doNoteOn) { return mParser->Parse(this, doNoteOn); }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
