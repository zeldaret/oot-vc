#include "revolution/hbm/nw4hbm/snd/ExternalSoundPlayer.h"
#include "revolution/hbm/nw4hbm/snd/SoundInstanceManager.h"
#include "revolution/hbm/nw4hbm/snd/SoundPlayer.h"

#include "revolution/hbm/nw4hbm/snd/SeqPlayer.h"
#include "revolution/hbm/nw4hbm/snd/SeqSound.h"
#include "revolution/hbm/nw4hbm/snd/StrmSound.h"
#include "revolution/hbm/nw4hbm/snd/WaveSound.h"

#include "decomp.h"

namespace nw4hbm {
namespace snd {

SoundPlayer::SoundPlayer() : mPlayableCount(1), mPlayableLimit(1), mUsePlayerHeap(false) { InitParam(); }

SoundPlayer::~SoundPlayer() { StopAllSound(0); }

void SoundPlayer::InitParam() {
    mVolume = 1.0f;
    mOutputLineFlag = OUTPUT_LINE_MAIN;
    mOutputLineFlagEnable = false;

    mMainOutVolume = 1.0f;
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mRemoteOutVolume[i] = 1.0f;
    }
}

DECOMP_FORCE_CLASS_METHOD(detail::PlayerHeapList, PushBack(nullptr));

void SoundPlayer::Update() {
    for (detail::BasicSoundPlayerPlayList::Iterator it = mSoundList.GetBeginIter(); it != mSoundList.GetEndIter();) {
        detail::BasicSoundPlayerPlayList::Iterator currIt = it++;
        currIt->Update();
    }
    detail_SortPriorityList();
}

void SoundPlayer::StopAllSound(int frames) {
    for (detail::BasicSoundPlayerPlayList::Iterator it = mSoundList.GetBeginIter(); it != mSoundList.GetEndIter();) {
        detail::BasicSoundPlayerPlayList::Iterator currIt = it++;
        currIt->Stop(frames);
    }
}

void SoundPlayer::SetVolume(f32 volume) {
    NW4HBMAssert_Line(volume >= 0.0f, 153);
    mVolume = volume;
}

int SoundPlayer::detail_GetOutputLine() const { return mOutputLineFlag; }

bool SoundPlayer::detail_IsEnabledOutputLine() const { return mOutputLineFlagEnable; }

f32 SoundPlayer::detail_GetRemoteOutVolume(int remoteIndex) const {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 259);
    return mRemoteOutVolume[remoteIndex];
}

void SoundPlayer::detail_RemoveSoundList(detail::BasicSound* sound) {
    mSoundList.Erase(sound);
    sound->SetSoundPlayer(nullptr);
}

void SoundPlayer::detail_InsertPriorityList(detail::BasicSound* sound) {
    detail::BasicSoundPlayerPrioList::Iterator it = mPriorityList.GetBeginIter();

    for (; it != mPriorityList.GetEndIter(); it++) {
        if (sound->CalcCurrentPlayerPriority() < it->CalcCurrentPlayerPriority()) {
            break;
        }
    }

    mPriorityList.Insert(it, sound);
}

void SoundPlayer::detail_RemovePriorityList(detail::BasicSound* sound) { mPriorityList.Erase(sound); }

void SoundPlayer::detail_SortPriorityList() {
    detail::BasicSoundPlayerPrioList listsByPrio[detail::BasicSound::PRIORITY_MAX + 1];

    while (!mPriorityList.IsEmpty()) {
        detail::BasicSound& rSound = mPriorityList.GetFront();
        mPriorityList.PopFront();
        listsByPrio[rSound.CalcCurrentPlayerPriority()].PushBack(&rSound);
    }

    for (int i = 0; i < detail::BasicSound::PRIORITY_MAX + 1; i++) {
        while (!listsByPrio[i].IsEmpty()) {
            detail::BasicSound& rSound = listsByPrio[i].GetFront();
            listsByPrio[i].PopFront();
            mPriorityList.PushBack(&rSound);
        }
    }
}

void SoundPlayer::detail_InsertSoundList(detail::BasicSound* sound) {
    mSoundList.PushBack(sound);
    sound->SetSoundPlayer(this);
}

detail::SeqSound* SoundPlayer::detail_AllocSeqSound(int priority, int startPriority,
                                                    detail::BasicSound::AmbientArgInfo* ambientArgInfo,
                                                    detail::ExternalSoundPlayer* extPlayer, u32 id,
                                                    detail::SoundInstanceManager<detail::SeqSound>* manager) {
    if (manager == nullptr) {
        return nullptr;
    }

    int priorityReduction = CalcPriorityReduction(ambientArgInfo, id);

    startPriority = ut::Clamp(startPriority - priorityReduction, 0, detail::BasicSound::PRIORITY_MAX);

    if (!CheckPlayableSoundCount(startPriority, extPlayer)) {
        return nullptr;
    }

    detail::SeqSound* sound = manager->Alloc(startPriority);
    if (sound == nullptr) {
        return nullptr;
    }

    detail_AllocPlayerHeap(sound);

    if (ambientArgInfo != nullptr) {
        InitAmbientArg(sound, ambientArgInfo);
    }

    sound->SetPriority(priority);
    sound->GetAmbientParam().priority = priorityReduction;

    detail_InsertSoundList(sound);

    if (extPlayer != nullptr) {
        extPlayer->InsertSoundList(sound);
    }

    detail_InsertPriorityList(sound);

    return sound;
}

detail::StrmSound* SoundPlayer::detail_AllocStrmSound(int priority, int startPriority,
                                                      detail::BasicSound::AmbientArgInfo* ambientArgInfo,
                                                      detail::ExternalSoundPlayer* extPlayer, u32 id,
                                                      detail::SoundInstanceManager<detail::StrmSound>* manager) {
    if (manager == nullptr) {
        return nullptr;
    }

    int priorityReduction = CalcPriorityReduction(ambientArgInfo, id);

    startPriority = ut::Clamp(startPriority - priorityReduction, 0, detail::BasicSound::PRIORITY_MAX);

    if (!CheckPlayableSoundCount(startPriority, extPlayer)) {
        return nullptr;
    }

    detail::StrmSound* sound = manager->Alloc(startPriority);
    if (sound == nullptr) {
        return nullptr;
    }

    detail_AllocPlayerHeap(sound);

    if (ambientArgInfo != nullptr) {
        InitAmbientArg(sound, ambientArgInfo);
    }

    sound->SetPriority(priority);
    sound->GetAmbientParam().priority = priorityReduction;

    detail_InsertSoundList(sound);

    if (extPlayer != nullptr) {
        extPlayer->InsertSoundList(sound);
    }

    detail_InsertPriorityList(sound);

    return sound;
}

detail::WaveSound* SoundPlayer::detail_AllocWaveSound(int priority, int startPriority,
                                                      detail::BasicSound::AmbientArgInfo* ambientArgInfo,
                                                      detail::ExternalSoundPlayer* extPlayer, u32 id,
                                                      detail::SoundInstanceManager<detail::WaveSound>* manager) {
    if (manager == nullptr) {
        return nullptr;
    }

    int priorityReduction = CalcPriorityReduction(ambientArgInfo, id);

    startPriority = ut::Clamp(startPriority - priorityReduction, 0, detail::BasicSound::PRIORITY_MAX);

    if (!CheckPlayableSoundCount(startPriority, extPlayer)) {
        return nullptr;
    }

    detail::WaveSound* sound = manager->Alloc(startPriority);
    if (sound == nullptr) {
        return nullptr;
    }

    detail_AllocPlayerHeap(sound);

    if (ambientArgInfo != nullptr) {
        InitAmbientArg(sound, ambientArgInfo);
    }

    sound->SetPriority(priority);
    sound->GetAmbientParam().priority = priorityReduction;

    detail_InsertSoundList(sound);

    if (extPlayer != nullptr) {
        extPlayer->InsertSoundList(sound);
    }

    detail_InsertPriorityList(sound);

    return sound;
}

void SoundPlayer::SetPlayableSoundCount(int count) {
    NW4HBMAssert_Line(count >= 0, 527);
    mPlayableCount = count;

    if (mUsePlayerHeap) {
        NW4HBMCheckMessage_Line(mPlayableCount <= mPlayableLimit, 535, "playable sound count is over limit.");
        mPlayableCount = ut::Clamp<u16>(mPlayableCount, 0, mPlayableLimit);
    }

    while (GetPlayingSoundCount() > GetPlayableSoundCount()) {
        detail::BasicSound* dropSound = detail_GetLowestPrioritySound();
        NW4HBMAssertPointerNonnull_Line(dropSound, 544);

        dropSound->Shutdown();
    }
}

void SoundPlayer::detail_SetPlayableSoundLimit(int limit) {
    NW4HBMAssert_Line(limit >= 0, 551);
    mPlayableLimit = limit;
}

bool SoundPlayer::CheckPlayableSoundCount(int startPriority, detail::ExternalSoundPlayer* extPlayer) {
    if (GetPlayableSoundCount() == 0) {
        return false;
    }

    while (GetPlayingSoundCount() >= GetPlayableSoundCount()) {
        detail::BasicSound* pDrosound = detail_GetLowestPrioritySound();

        if (pDrosound == nullptr) {
            return false;
        }

        if (startPriority < pDrosound->CalcCurrentPlayerPriority()) {
            return false;
        }

        pDrosound->Shutdown();
    }

    if (extPlayer != nullptr) {
        if (extPlayer->GetPlayableSoundCount() == 0) {
            return false;
        }

        while (extPlayer->GetPlayingSoundCount() >= extPlayer->GetPlayableSoundCount()) {
            detail::BasicSound* pDrosound = extPlayer->GetLowestPrioritySound();

            if (pDrosound == nullptr) {
                return false;
            }

            if (startPriority < pDrosound->CalcCurrentPlayerPriority()) {
                return false;
            }

            pDrosound->Shutdown();
        }
    }

    return true;
}

int SoundPlayer::CalcPriorityReduction(detail::BasicSound::AmbientArgInfo* ambientArgInfo, u32 id) {
    int priority = 0;

    if (ambientArgInfo != nullptr) {
        detail::SoundParam param;

        ambientArgInfo->paramUpdateCallback->detail_Update(
            &param, id, nullptr, ambientArgInfo->arg,
            detail::BasicSound::AmbientParamUpdateCallback::PARAM_UPDATE_PRIORITY);

        priority = param.priority;
    }

    return priority;
}

void SoundPlayer::InitAmbientArg(detail::BasicSound* sound, detail::BasicSound::AmbientArgInfo* ambientArgInfo) {
    if (ambientArgInfo == nullptr) {
        return;
    }

    void* pExtArg = ambientArgInfo->argAllocaterCallback->detail_AllocAmbientArg(ambientArgInfo->argSize);

    if (pExtArg == nullptr) {
        return;
    }

    memcpy(pExtArg, ambientArgInfo->arg, ambientArgInfo->argSize);

    sound->SetAmbientParamCallback(ambientArgInfo->paramUpdateCallback, ambientArgInfo->argUpdateCallback,
                                   ambientArgInfo->argAllocaterCallback, pExtArg);
}

detail::PlayerHeap* SoundPlayer::detail_AllocPlayerHeap(detail::BasicSound* sound) {
    if (mHeapList.IsEmpty()) {
        return nullptr;
    }

    detail::PlayerHeap& rHeap = mHeapList.GetFront();
    mHeapList.PopFront();

    rHeap.SetSound(sound);
    sound->SetPlayerHeap(&rHeap);
    rHeap.Clear();

    return &rHeap;
}

// THIS
void SoundPlayer::detail_AppendPlayerHeap(detail::PlayerHeap* heap) {
    NW4HBMAssertPointerNonnull_Line(heap, 598);
    heap->SetSoundPlayer(this);
    mHeapList.PushBack(heap);

    mUsePlayerHeap = true;
}

void SoundPlayer::detail_FreePlayerHeap(detail::BasicSound* sound) {
    NW4HBMAssertPointerNonnull_Line(sound, 640);
    detail::PlayerHeap* pHeap = sound->GetPlayerHeap();

    if (pHeap != nullptr) {
        mHeapList.PushBack(pHeap);
    }

    if (pHeap != nullptr) {
        pHeap->SetSound(nullptr);
    }

    sound->SetPlayerHeap(nullptr);
}

} // namespace snd
} // namespace nw4hbm
