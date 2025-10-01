#ifndef NW4R_SND_WAVE_SOUND_H
#define NW4R_SND_WAVE_SOUND_H

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/BasicSound.h"
#include "revolution/hbm/nw4hbm/snd/WsdPlayer.h"
#include "revolution/hbm/nw4hbm/snd/debug.h"

#include "revolution/hbm/nw4hbm/ut/LinkList.h"

namespace nw4hbm {
namespace snd {
class WaveSoundHandle;
namespace detail {
template <typename T> class SoundInstanceManager;

class WaveSound : public BasicSound {
    friend class WaveSoundHandle;

  public:
    NW4HBM_UT_RUNTIME_TYPEINFO;

    explicit WaveSound(SoundInstanceManager<WaveSound>* manager);

    /* 0x28 */ virtual void Shutdown();

    /* 0x4C */ virtual void SetPlayerPriority(int priority);
    /* 0x5C */ virtual bool IsAttachedTempSpecialHandle();
    /* 0x60 */ virtual void DetachTempSpecialHandle();

    virtual BasicPlayer& GetBasicPlayer() { return mWsdPlayer; } // 0x68
    virtual const BasicPlayer& GetBasicPlayer() const { return mWsdPlayer; } // 0x6C

    bool Prepare(const void* waveSoundBase, s32 waveSoundOffset, int voices, const WsdTrack::WsdCallback* callback,
                 u32 callbackData);

    void SetChannelPriority(int priority);
    void SetReleasePriorityFix(bool flag);

  private:
    /* 0xD8 */ WsdPlayer mWsdPlayer;
    /* 0x1B0 */ WaveSoundHandle* mTempSpecialHandle;
    /* 0x1B4 */ SoundInstanceManager<WaveSound>* mManager;
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
