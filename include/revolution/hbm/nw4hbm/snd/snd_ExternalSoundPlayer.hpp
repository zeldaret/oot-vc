#ifndef NW4R_SND_EXTERNAL_SOUND_PLAYER_H
#define NW4R_SND_EXTERNAL_SOUND_PLAYER_H
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/snd_BasicSound.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

class ExternalSoundPlayer {
  public:
    ExternalSoundPlayer();
    ~ExternalSoundPlayer();

    int GetPlayableSoundCount() const { return mPlayableCount; }
    void SetPlayableSoundCount(int count);

    int GetPlayingSoundCount() const { return mSoundList.GetSize(); }

    f32 detail_GetVolume() const { return mVolume; }
    BasicSound* GetLowestPrioritySound();

    void InsertSoundList(BasicSound* pSound);
    void RemoveSoundList(BasicSound* pSound);

    bool detail_CanPlaySound(int startPriority);
    bool AppendSound(BasicSound *sound);

    template <typename TForEachFunc> TForEachFunc ForEachSound(TForEachFunc pFunc, bool reverse) {
        if (reverse) {
            BasicSoundExtPlayList::RevIterator it = mSoundList.GetBeginReverseIter();

            while (it != mSoundList.GetEndReverseIter()) {
                BasicSoundExtPlayList::RevIterator curr = it;

                SoundHandle handle;
                handle.detail_AttachSoundAsTempHandle(&*curr);
                pFunc(handle);

                if (handle.IsAttachedSound()) {
                    ++it;
                }
            }
        } else {
            NW4R_UT_LINKLIST_FOREACH_SAFE(it, mSoundList, {
                SoundHandle handle;
                handle.detail_AttachSoundAsTempHandle(&*it);
                pFunc(handle);
            });
        }

        return pFunc;
    }

  private:
    BasicSound::ExtSoundPlayerPlayLinkList mSoundList; // at 0x0
    u16 mPlayableCount; // at 0xC
    f32 mVolume; // at 0x10
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
