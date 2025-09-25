#include "decomp.h"
#include "revolution/hbm/nw4hbm/snd/ExternalSoundPlayer.h"

namespace nw4hbm {
namespace snd {
namespace detail {

// not sure which one uses this exactly, maybe StopAllSound?
DECOMP_FORCE_CLASS_METHOD(BasicSoundExtPlayList, GetPointerFromNode(static_cast<ut::LinkListNode*>(nullptr)));

void ExternalSoundPlayer::InsertSoundList(BasicSound* sound) {
    mSoundList.PushBack(sound);
    sound->SetExternalSoundPlayer(this);
}

void ExternalSoundPlayer::RemoveSoundList(BasicSound* sound) {
    mSoundList.Erase(sound);
    sound->SetExternalSoundPlayer(nullptr);
}

BasicSound* ExternalSoundPlayer::GetLowestPrioritySound() {
    int lowestPrio = BasicSound::PRIORITY_MAX + 1;
    BasicSound* pLowest = nullptr;

    for (BasicSoundExtPlayList::Iterator it = mSoundList.GetBeginIter(); it != mSoundList.GetEndIter(); it++) {

        int priority = it->CalcCurrentPlayerPriority();

        if (lowestPrio > priority) {
            pLowest = &*it;
            lowestPrio = priority;
        }
    }

    return pLowest;
}
} // namespace detail
} // namespace snd
} // namespace nw4hbm
