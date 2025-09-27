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

    virtual void Shutdown(); // 0x28

    virtual void SetPlayerPriority(int priority); // 0x4C
    virtual bool IsAttachedTempSpecialHandle(); // 0x5C
    virtual void DetachTempSpecialHandle(); // 0x60

    virtual BasicPlayer& GetBasicPlayer() { return mWsdPlayer; } // 0x68
    virtual const BasicPlayer& GetBasicPlayer() const { return mWsdPlayer; } // 0x6C

    bool Prepare(const void* waveSoundBase, s32 waveSoundOffset, int voices, const WsdTrack::WsdCallback* callback,
                 u32 callbackData);

    void SetChannelPriority(int priority);
    void SetReleasePriorityFix(bool flag);

  private:
    WsdPlayer mWsdPlayer; // 0xD8
    WaveSoundHandle* mTempSpecialHandle; // 0x1B0
    SoundInstanceManager<WaveSound>* mManager; // 0x1B4
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_WAVE_SOUND_H
