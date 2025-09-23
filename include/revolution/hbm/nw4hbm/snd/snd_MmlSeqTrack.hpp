#ifndef NW4R_SND_MML_SEQ_TRACK_H
#define NW4R_SND_MML_SEQ_TRACK_H

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/snd/snd_SeqTrack.hpp"

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
// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x31308
class MmlSeqTrack : public SeqTrack {
    // methods
  public:
    // cdtors
    MmlSeqTrack();

    // virtual function ordering
    // vtable SeqTrack
    virtual ParseResult Parse(bool doNoteOn);

    // methods
    void SetMmlParser(MmlParser const* parser) { mParser = parser; }

    // members
  private:
    /* base SeqTrack */ // size 0xc8, offset 0x00
    MmlParser const* mParser; // size 0x04, offset 0xc8
}; // size 0xcc
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_MML_SEQ_TRACK_H
