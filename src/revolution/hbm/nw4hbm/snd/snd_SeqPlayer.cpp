#include "revolution/hbm/nw4hbm/snd/SeqPlayer.h"
#include "revolution/hbm/nw4hbm/snd/SeqTrack.h"
#include "revolution/hbm/nw4hbm/snd/SeqTrackAllocator.h"
#include "revolution/hbm/nw4hbm/snd/NoteOnCallback.h"
#include "revolution/hbm/ut.hpp"
#include "decomp.h"

namespace nw4hbm {
namespace snd {
namespace detail {

namespace {
SeqPlayerList sPlayerList;
}
vs16 SeqPlayer::mGlobalVariable[GLOBAL_VARIABLE_NUM];

SeqPlayer::SeqPlayer() : mActiveFlag(false) {}

SeqPlayer::~SeqPlayer() {
    if (mActiveFlag) {
        FinishPlayer();
    }
}

void SeqPlayer::InitParam(int voices, NoteOnCallback* callback) {
    mPreparedFlag = false;
    mStartedFlag = false;
    mPauseFlag = false;
    mSkipFlag = false;
    mHomeButtonMenuFlag = false;
    mTempoRatio = 1.0f;
    mTempoCounter = 416;
    mExtVolume = 1.0f;
    mExtPitch = 1.0f;
    mExtPan = 0.0f;
    mExtSurroundPan = 0.0f;
    mExtPan2 = 0.0f;
    mExtSurroundPan2 = 0.0f;
    mPanRange = 1.0f;
    mExtLpfFreq = 0.0f;
    mOutputLineFlag = OUTPUT_LINE_MAIN;
    mMainSend = 0.0f;
    mMainOutVolume = 1.0f;

    for (int i = 0; i < AUX_BUS_NUM; i++) {
        mFxSend[i] = 0.0f;
    }
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mRemoteSend[i] = 0.0f;
        mRemoteFxSend[i] = 0.0f;
    }

    mTickCounter = 0;
    mVoiceOutCount = voices;

    mParserParam.tempo = DEFAULT_TEMPO;
    mParserParam.volume = 127;
    mParserParam.priority = 64;
    mParserParam.callback = callback;

    for (int i = 0; i < LOCAL_VARIABLE_NUM; i++) {
        mLocalVariable[i] = DEFAULT_VARIABLE_VALUE;
    }
    for (int i = 0; i < TRACK_NUM; i++) {
        mTracks[i] = nullptr;
    }
}

SeqPlayer::SetupResult SeqPlayer::Setup(SeqTrackAllocator* allocator, u32 allocTrackFlags, int voices,
                                        NoteOnCallback* callback) {
    ut::AutoInterruptLock lock;

    if (mActiveFlag) {
        FinishPlayer();
    }

    InitParam(voices, callback);
    {
        u32 trackFlags = allocTrackFlags;
        bool allocated = true;

        for (int i = 0; trackFlags != 0; trackFlags >>= 1, i++) {
            if (trackFlags & 1) {
                SeqTrack* track = allocator->AllocTrack(this);
                if (track == NULL) {
                    allocated = false;
                    break;
                }
                SetPlayerTrack(i, track);
            }
        }

        if (!allocated) {
            u32 trackFlags = allocTrackFlags;

            for (int i = 0; trackFlags != 0; trackFlags >>= 1, i++) {
                if (trackFlags & 1) {
                    SeqTrack* track = GetPlayerTrack(i);
                    if (track != NULL) {
                        allocator->FreeTrack(track);
                    }
                }
            }

            NW4HBMWarningMessage_Line(198, "Not enough SeqTrack.");
            return SETUP_ERR_CANNOT_ALLOCATE_TRACK;
        }
    }
    DisposeCallbackManager::GetInstance().RegisterDisposeCallback(this);

    mSeqTrackAllocator = allocator;
    mActiveFlag = true;

    return SETUP_SUCCESS;
}

void SeqPlayer::SetSeqData(const void* base, s32 offset) {
    SeqTrack* seqTrack = GetPlayerTrack(0);
    NW4HBMAssertPointerNonnull_Line(seqTrack, 224);

    if (base != NULL) {
        seqTrack->SetSeqData(base, offset);
    }

    mPreparedFlag = true;
}

DECOMP_FORCE(NW4HBMAssert_String(tempoRatio >= 0.0f));

bool SeqPlayer::Start() {
    if (!mPreparedFlag) {
        return false;
    }

    sPlayerList.PushBack(this);

    mHomeButtonMenuFlag = AxManager::GetInstance().IsHomeButtonMenu();
    mStartedFlag = true;

    return true;
}

void SeqPlayer::Stop() {
    if (mActiveFlag) {
        FinishPlayer();
    }
}

void SeqPlayer::Pause(bool flag) {
    ut::AutoInterruptLock lock;

    mPauseFlag = flag;

    for (int i = 0; i < TRACK_NUM; i++) {
        SeqTrack* track = GetPlayerTrack(i);

        if (track != NULL) {
            track->PauseAllChannel(flag);
        }
    }
}

void SeqPlayer::SetVolume(f32 volume) {
    NW4HBMAssert_Line(volume >= 0.0f, 349);
    ut::AutoInterruptLock lock;
    mExtVolume = volume;
}

void SeqPlayer::SetPitch(f32 pitch) {
    NW4HBMAssert_Line(pitch >= 0.0f, 356);
    ut::AutoInterruptLock lock;
    mExtPitch = pitch;
}

void SeqPlayer::SetPan(f32 pan) {
    ut::AutoInterruptLock lock;
    mExtPan = pan;
}

void SeqPlayer::SetSurroundPan(f32 surroundPan) {
    ut::AutoInterruptLock lock;
    mExtSurroundPan = surroundPan;
}

void SeqPlayer::SetPan2(f32 pan2) {
    ut::AutoInterruptLock lock;
    mExtPan2 = pan2;
}

void SeqPlayer::SetSurroundPan2(f32 surroundPan2) {
    ut::AutoInterruptLock lock;
    mExtSurroundPan2 = surroundPan2;
}

void SeqPlayer::SetLpfFreq(f32 lpfFreq) {
    ut::AutoInterruptLock lock;
    mExtLpfFreq = lpfFreq;
}

void SeqPlayer::SetMainOutVolume(f32 volume) {
    ut::AutoInterruptLock lock;
    mMainOutVolume = volume;
}

f32 SeqPlayer::GetMainOutVolume() const { return mMainOutVolume; }

void SeqPlayer::SetMainSend(f32 send) {
    ut::AutoInterruptLock lock;
    mMainSend = send;
}

f32 SeqPlayer::GetMainSend() const { return mMainSend; }

void SeqPlayer::SetFxSend(AuxBus bus, f32 send) {
    NW4HBMAssertHeaderClampedLValue_Line(bus, 0, 3, 429);
    ut::AutoInterruptLock lock;
    mFxSend[bus] = send;
}

f32 SeqPlayer::GetFxSend(AuxBus bus) const {
    NW4HBMAssertHeaderClampedLValue_Line(bus, 0, 3, 436);
    return mFxSend[bus];
}

void SeqPlayer::SetOutputLine(int lineFlag) {
    ut::AutoInterruptLock lock;
    mOutputLineFlag = lineFlag;
}

int SeqPlayer::GetOutputLine() const { return mOutputLineFlag; }

void SeqPlayer::SetRemoteOutVolume(int remoteIndex, f32 volume) {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 454);
    ut::AutoInterruptLock lock;
    mRemoteOutVolume[remoteIndex] = volume;
}

f32 SeqPlayer::GetRemoteOutVolume(int remoteIndex) const {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 461);
    return mRemoteOutVolume[remoteIndex];
}

void SeqPlayer::SetRemoteSend(int remoteIndex, f32 send) {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 467);
    ut::AutoInterruptLock lock;
    mRemoteSend[remoteIndex] = send;
}

f32 SeqPlayer::GetRemoteSend(int remoteIndex) const {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 474);
    return mRemoteSend[remoteIndex];
}

void SeqPlayer::SetRemoteFxSend(int remoteIndex, f32 send) {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 480);
    ut::AutoInterruptLock lock;
    mRemoteFxSend[remoteIndex] = send;
}

f32 SeqPlayer::GetRemoteFxSend(int remoteIndex) const {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 487);
    return mRemoteFxSend[remoteIndex];
}

void SeqPlayer::SetChannelPriority(int prio) {
    NW4HBMAssertHeaderClampedLRValue_Line(prio, 0, 127, 494);
    ut::AutoInterruptLock lock;
    mParserParam.priority = static_cast<u8>(prio);
}

DECOMP_FORCE(NW4HBMAssertHeaderClampedLValue_String(varNo));
DECOMP_FORCE(NW4HBMAssert_String(volume >= 0.0f));
DECOMP_FORCE(NW4HBMAssert_String(pitch >= 0.0f));
DECOMP_FORCE(&SeqTrack::SetMute);
DECOMP_FORCE(&SeqTrack::SetSilence);
DECOMP_FORCE(&SeqTrack::SetVolume);
DECOMP_FORCE(&SeqTrack::SetPitch);
DECOMP_FORCE(&SeqTrack::SetPan);
DECOMP_FORCE(&SeqTrack::SetSurroundPan);
DECOMP_FORCE(&SeqTrack::SetLpfFreq);
DECOMP_FORCE(&SeqTrack::SetPanRange);
DECOMP_FORCE(&SeqTrack::SetModDepth);
DECOMP_FORCE(&SeqTrack::SetModSpeed);
DECOMP_FORCE(&SeqTrack::SetMainSend);
DECOMP_FORCE(&SeqTrack::SetFxSend);
DECOMP_FORCE(&SeqTrack::SetRemoteSend);
DECOMP_FORCE(&SeqTrack::SetRemoteFxSend);

void SeqPlayer::InvalidateData(const void* start, const void* end) {
    ut::AutoInterruptLock lock;

    if (mActiveFlag) {
        for (int i = 0; i < TRACK_NUM; i++) {
            SeqTrack* track = GetPlayerTrack(i);
            if (track == NULL) {
                continue;
            }

            const u8* base = track->GetParserTrackParam().baseAddr;
            if (start <= base && base <= end) {
                FinishPlayer();
                break;
            }
        }
    }
}

void SeqPlayer::CloseTrack(int trackNo) {
    NW4HBMAssertHeaderClampedLValue_Line(trackNo, 0, 16, 765);
    SeqTrack* track = GetPlayerTrack(trackNo);
    if (track == NULL) {
        return;
    }

    track->Close();

    mSeqTrackAllocator->FreeTrack(mTracks[trackNo]);
    mTracks[trackNo] = nullptr;
}

SeqTrack* SeqPlayer::GetPlayerTrack(int trackNo) {
    if (trackNo > TRACK_NUM - 1) {
        return nullptr;
    }

    return mTracks[trackNo];
}

void SeqPlayer::SetPlayerTrack(int trackNo, SeqTrack* track) {
    if (trackNo > TRACK_NUM - 1) {
        return;
    }

    mTracks[trackNo] = track;
    track->SetPlayerTrackNo(trackNo);
}

void SeqPlayer::FinishPlayer() {
    {
        ut::AutoInterruptLock lock;

        if (mStartedFlag) {
            sPlayerList.Erase(this);
        }
        if (mActiveFlag) {
            DisposeCallbackManager::GetInstance().UnregisterDisposeCallback(this);
        }
    }

    for (int i = 0; i < TRACK_NUM; i++) {
        CloseTrack(i);
    }
    mActiveFlag = false;
}

int SeqPlayer::ParseNextTick(bool doNoteOn) {
    bool active = false;

    for (int i = 0; i < TRACK_NUM; i++) {
        SeqTrack* track = GetPlayerTrack(i);
        if (track == NULL) {
            continue;
        }

        track->UpdateChannelLength();

        if (track->ParseNextTick(doNoteOn) == 0) {
            active = true;
        } else {
            CloseTrack(i);
        }
    }

    if (!active) {
        return 1;
    }

    return 0;
}

vs16* SeqPlayer::GetVariablePtr(int varNo) {
    NW4HBMAssertHeaderClampedLRValue_Line(varNo, 0, 32, 896);

    if (varNo < LOCAL_VARIABLE_NUM) {
        return &mLocalVariable[varNo];
    }

    if (varNo < VARIABLE_NUM) {
        return &mGlobalVariable[varNo - LOCAL_VARIABLE_NUM];
    }

    return nullptr;
}

int SeqPlayer::UpdateTempoCounter() {
    int tick = 0;
    while (mTempoCounter >= 416) {
        mTempoCounter -= 416;
        tick++;
    }

    f32 tempo = mParserParam.tempo;
    tempo *= mTempoRatio;

    mTempoCounter += static_cast<int>(tempo);

    return tick;
}

void SeqPlayer::Update() {
    NW4HBMAssert_Line(mActiveFlag, 920);
    NW4HBMAssert_Line(mStartedFlag, 921);

    if (!mActiveFlag) {
        return;
    }

    if (!mStartedFlag) {
        return;
    }

    if (!mPauseFlag && !mSkipFlag) {
        if (mHomeButtonMenuFlag || !AxManager::GetInstance().IsHomeButtonMenu()) {
            for (int tick = UpdateTempoCounter(); tick > 0; tick--) {
                if (ParseNextTick(true) != 0) {
                    FinishPlayer();
                    break;
                }
                mTickCounter++;
            }
        }
    }
    UpdateChannelParam();
}

void SeqPlayer::UpdateChannelParam() {
    SeqTrack* track;

    for (int trackNo = 0; trackNo < TRACK_NUM; trackNo++) {
        track = GetPlayerTrack(trackNo);
        if (track != NULL) {
            track->UpdateChannelParam();
        }
    }
}

void SeqPlayer::UpdateAllPlayers() {
    for (SeqPlayerList::Iterator it = sPlayerList.GetBeginIter(); it != sPlayerList.GetEndIter();) {
        SeqPlayerList::Iterator currIt = it++;
        currIt->Update();
    }
}

void SeqPlayer::StopAllPlayers() {
    for (SeqPlayerList::Iterator it = sPlayerList.GetBeginIter(); it != sPlayerList.GetEndIter();) {
        SeqPlayerList::Iterator currIt = it++;
        currIt->Stop();
    }

    NW4HBMAssert_Line(sPlayerList.IsEmpty(), 983);
}

Channel* SeqPlayer::NoteOn(int bankNo, const NoteOnInfo& noteOnInfo) {
    return mParserParam.callback->NoteOn(this, bankNo, noteOnInfo);
}

// Blank. Not sure if this was explicitly defined here but if the function is defined in the header, it would link to
// MidiSeqPlayer instead.
void SeqPlayer::ChannelCallback(Channel* channel) {}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
