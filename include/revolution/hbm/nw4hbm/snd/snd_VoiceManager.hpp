#ifndef NW4R_SND_VOICE_MANAGER_H
#define NW4R_SND_VOICE_MANAGER_H
#include "revolution/types.h"
#include "macros.h"

#include "revolution/hbm/nw4hbm/snd/snd_Voice.hpp"

#include "revolution/ax.h"

namespace nw4hbm {
namespace snd {
namespace detail {

class VoiceManager {
  public:
    static const int VOICE_MAX = AX_VOICE_MAX;
    static const int WORK_SIZE_MAX = VOICE_MAX * sizeof(Voice);

  public:
    static VoiceManager& GetInstance();

    u32 GetRequiredMemSize();

    void Setup(void* pBuffer, u32 size);
    void Shutdown();
    void StopAllVoices();

    Voice* AllocVoice(int channels, int voices, int priority, Voice::VoiceCallback pCallback, void* pCallbackArg);
    void FreeVoice(Voice* pVoice);

    void UpdateAllVoices();
    void NotifyVoiceUpdate();
    void ChangeVoicePriority(Voice* pVoice);
    void UpdateAllVoicesSync(u32 syncFlag) {
        ut::AutoInterruptLock lock;

        NW4HBM_RANGE_FOR_NO_AUTO_INC(itr, mPrioVoiceList) {
            DECLTYPE(itr) curItr = itr++;

            if (curItr->mActiveFlag) {
                curItr->mSyncFlag |= syncFlag;
            }
        }
    }

    const VoiceList& GetVoiceList() const { return mPrioVoiceList; }

  private:
    VoiceManager();

    void AppendVoiceList(Voice* pVoice);
    void RemoveVoiceList(Voice* pVoice);

    void UpdateEachVoicePriority(const VoiceList::Iterator& rBegin, const VoiceList::Iterator& rEnd);

    int DropLowestPriorityVoice(int priority);

  private:
    bool mInitialized; // at 0x0
    VoiceList mPrioVoiceList; // at 0x4
    VoiceList mFreeVoiceList; // at 0x10
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
