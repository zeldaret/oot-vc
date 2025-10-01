#ifndef NW4R_SND_MIDI_SEQ_PLAYER_H
#define NW4R_SND_MIDI_SEQ_PLAYER_H

#include "revolution/hbm/nw4hbm/snd/SeqPlayer.h"

namespace nw4hbm {
namespace snd {
namespace detail {

// No midi for you
class MidiSeqPlayer : public SeqPlayer {
    MidiSeqPlayer();
    virtual ~MidiSeqPlayer() {}
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
