#ifndef NW4R_SND_PLAYER_HEAP_H
#define NW4R_SND_PLAYER_HEAP_H

#include "revolution/hbm/nw4hbm/snd/SoundHeap.h"
#include "revolution/hbm/nw4hbm/snd/SoundMemoryAllocatable.h"

namespace nw4hbm {
namespace snd {
namespace detail {
class BasicSound;
}
} // namespace snd
} // namespace nw4hbm
namespace nw4hbm {
namespace snd {
class SoundPlayer;
}
} // namespace nw4hbm

namespace nw4hbm {
namespace snd {
class SoundPlayer;
namespace detail {
class BasicSound;
class PlayerHeap : public SoundHeap {
  public:
    PlayerHeap() : mSound(nullptr), mPlayer(nullptr) {}
    virtual ~PlayerHeap() {} // 0x08

    void SetSound(BasicSound* sound) { mSound = sound; }
    void SetSoundPlayer(SoundPlayer* player) { mPlayer = player; }

  public:
    /* 0x2C */ ut::LinkListNode mLink;

  private:
    /* 0x34 */ BasicSound* mSound;
    /* 0x38 */ SoundPlayer* mPlayer;
};
typedef ut::LinkList<PlayerHeap, offsetof(PlayerHeap, mLink)> PlayerHeapList;
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
