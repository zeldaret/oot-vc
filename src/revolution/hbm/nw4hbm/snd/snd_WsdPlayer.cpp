#define MAKE_DTOR_ZERO
#include "revolution/hbm/nw4hbm/snd/WsdPlayer.h"
#undef MAKE_DTOR_ZERO

#include "revolution/hbm/nw4hbm/snd/AxManager.h"

#include "revolution/hbm/nw4hbm/snd/DisposeCallbackManager.h"

#include "revolution/hbm/ut.hpp"

#include "decomp.h"

namespace nw4hbm {
namespace snd {
namespace detail {

namespace {
WsdPlayerList sPlayerList;
}

WsdPlayer::WsdPlayer() : BasicPlayer(), DisposeCallback(), mPlayerLink(), mActiveFlag(false), mTrack() {}

void WsdPlayer::InitParam(int voices, const WsdTrack::WsdCallback* callback, u32 callbackData) {
    mPreparedFlag = false;
    mStartedFlag = false;
    mPauseFlag = false;
    mSkipFlag = false;
    mHomeButtonMenuFlag = false;
    mExtVolume = 1.0f;
    mExtPitch = 1.0f;
    mExtPan = 0.0f;
    mExtSurroundPan = 0.0f;
    mExtPan2 = 0.0f;
    mExtSurroundPan2 = 0.0f;
    mPanRange = 1.0f;
    mExtLpfFreq = 0.0f;
    mOutputLineFlag = OUTPUT_LINE_MAIN;
    mMainOutVolume = 1.0f;
    mMainSend = 0.0f;
    for (int i = 0; i < AUX_BUS_NUM; i++) {
        mFxSend[i] = 0.0f;
    }
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mRemoteSend[i] = 0.0f;
        mRemoteFxSend[i] = 0.0f;
    }
    mTickCounter = 0;
    mVoiceOutCount = voices;
    mPriority = 64;
    mCallback = callback;
    mCallbackData = callbackData;
}

bool WsdPlayer::Prepare(const void* waveSoundBase, int index, int voices, const WsdTrack::WsdCallback* callback,
                        u32 callbackData) {
    ut::AutoInterruptLock lock;

    if (mActiveFlag) {
        FinishPlayer();
    }

    InitParam(voices, callback, callbackData);

    mTrack.Init(this);

    mTrack.Start(waveSoundBase, index);

    DisposeCallbackManager::GetInstance().RegisterDisposeCallback(this);

    mActiveFlag = true;
    mPreparedFlag = true;

    return true;
}

bool WsdPlayer::Start() {
    if (!mPreparedFlag) {
        return false;
    }
    sPlayerList.PushBack(this);
    mHomeButtonMenuFlag = AxManager::GetInstance().IsHomeButtonMenu();
    mStartedFlag = true;

    return true;
}

void WsdPlayer::Stop() {
    if (mActiveFlag) {
        FinishPlayer();
    }
}

void WsdPlayer::Pause(bool flag) {
    ut::AutoInterruptLock lock;

    mPauseFlag = static_cast<u8>(flag);
    mTrack.PauseAllChannel(flag);
}

void WsdPlayer::SetVolume(f32 volume) {
    NW4HBMAssert_Line(volume >= 0.0f, 250);
    ut::AutoInterruptLock lock;
    mExtVolume = volume;
}

void WsdPlayer::SetPitch(f32 pitch) {
    NW4HBMAssert_Line(pitch >= 0.0f, 257);
    ut::AutoInterruptLock lock;
    mExtPitch = pitch;
}

void WsdPlayer::SetPan(f32 pan) {
    ut::AutoInterruptLock lock;
    mExtPan = pan;
}

void WsdPlayer::SetSurroundPan(f32 surroundPan) {
    ut::AutoInterruptLock lock;
    mExtSurroundPan = surroundPan;
}

void WsdPlayer::SetPan2(f32 pan2) {
    ut::AutoInterruptLock lock;
    mExtPan2 = pan2;
}

void WsdPlayer::SetSurroundPan2(f32 surroundPan2) {
    ut::AutoInterruptLock lock;
    mExtSurroundPan2 = surroundPan2;
}

void WsdPlayer::SetLpfFreq(f32 lpfFreq) {
    ut::AutoInterruptLock lock;
    mExtLpfFreq = lpfFreq;
}

void WsdPlayer::SetMainSend(f32 send) {
    ut::AutoInterruptLock lock;
    mMainSend = send;
}

f32 WsdPlayer::GetMainSend() const { return mMainSend; }

void WsdPlayer::SetFxSend(AuxBus bus, f32 send) {
    NW4HBMAssertHeaderClampedLValue_Line(bus, 0, 3, 319);
    mFxSend[bus] = send;
}

f32 WsdPlayer::GetFxSend(AuxBus bus) const {
    NW4HBMAssertHeaderClampedLValue_Line(bus, 0, 3, 325);
    return mFxSend[bus];
}

void WsdPlayer::SetOutputLine(int lineFlag) {
    ut::AutoInterruptLock lock;
    mOutputLineFlag = lineFlag;
}

int WsdPlayer::GetOutputLine() const { return mOutputLineFlag; }

void WsdPlayer::SetRemoteOutVolume(int remoteIndex, f32 volume) {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 343);
    ut::AutoInterruptLock lock;
    mRemoteOutVolume[remoteIndex] = volume;
}

f32 WsdPlayer::GetRemoteOutVolume(int remoteIndex) const {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 350);
    return mRemoteOutVolume[remoteIndex];
}

void WsdPlayer::SetRemoteSend(int remoteIndex, f32 send) {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 356);
    ut::AutoInterruptLock lock;
    mRemoteSend[remoteIndex] = send;
}

f32 WsdPlayer::GetRemoteSend(int remoteIndex) const {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 363);
    return mRemoteSend[remoteIndex];
}

void WsdPlayer::SetRemoteFxSend(int remoteIndex, f32 send) {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 369);
    ut::AutoInterruptLock lock;
    mRemoteFxSend[remoteIndex] = send;
}

f32 WsdPlayer::GetRemoteFxSend(int remoteIndex) const {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 376);
    return mRemoteFxSend[remoteIndex];
}

void WsdPlayer::SetMainOutVolume(f32 volume) {
    ut::AutoInterruptLock lock;
    mMainOutVolume = volume;
}

f32 WsdPlayer::GetMainOutVolume() const { return mMainOutVolume; }

void WsdPlayer::SetChannelPriority(int prio) {
    NW4HBMAssertHeaderClampedLRValue_Line(prio, 0, 127, 394);
    ut::AutoInterruptLock lock;
    mPriority = static_cast<u8>(prio);
}

DECOMP_FORCE(NW4HBMAssert_String(trackNo == 0));

void WsdPlayer::InvalidateData(const void* start, const void* end) {
    ut::AutoInterruptLock lock;

    if (mActiveFlag) {
        const void* current = mTrack.GetWsdDataAddress();
        if (start <= current && current <= end) {
            FinishPlayer();
        }
    }
}

void WsdPlayer::FinishPlayer() {
    {
        ut::AutoInterruptLock lock;

        if (mStartedFlag) {
            sPlayerList.Erase(this);
        }

        if (mActiveFlag) {
            DisposeCallbackManager::GetInstance().UnregisterDisposeCallback(this);
        }
    }

    mTrack.Close();
    mActiveFlag = false;
}

BOOL WsdPlayer::ParseNextTick(bool doNoteOn) {
    bool active = false;

    if (mTrack.ParseNextTick(mCallback, mCallbackData, doNoteOn) >= 0) {
        active = true;
    } else {
        mTrack.Close();
    }

    if (!active) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void WsdPlayer::UpdateChannel() { mTrack.UpdateChannel(); }

void WsdPlayer::Update() {
    NW4HBMAssert_Line(mActiveFlag, 534);
    NW4HBMAssert_Line(mStartedFlag, 535);
    if (!mActiveFlag) {
        return;
    }

    if (!mStartedFlag) {
        return;
    }

    if (!mPauseFlag && !mSkipFlag) {
        if (mHomeButtonMenuFlag || !AxManager::GetInstance().IsHomeButtonMenu()) {
            if (ParseNextTick(true) != 0) {
                FinishPlayer();
                return;
            }
        }
    }

    UpdateChannel();
}

void WsdPlayer::UpdateAllPlayers() {
    for (WsdPlayerList::Iterator it = sPlayerList.GetBeginIter(); it != sPlayerList.GetEndIter();) {
        WsdPlayerList::Iterator currIt = it++;
        currIt->Update();
    }
}

void WsdPlayer::StopAllPlayers() {
    for (WsdPlayerList::Iterator it = sPlayerList.GetBeginIter(); it != sPlayerList.GetEndIter();) {
        WsdPlayerList::Iterator currIt = it++;
        currIt->Stop();
    }

    NW4HBMAssert_Line(sPlayerList.IsEmpty(), 589);
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
