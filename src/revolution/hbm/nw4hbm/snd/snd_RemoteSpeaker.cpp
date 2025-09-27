#include "revolution/hbm/nw4hbm/snd/RemoteSpeaker.h"
#include "revolution/hbm/nw4hbm/ut.h"

#include "revolution/os.h"
#include "revolution/wenc.h"
#include "revolution/wpad.h"

#include "cstring.hpp"

namespace nw4hbm {
namespace snd {

RemoteSpeaker::RemoteSpeaker()
    : mInitFlag(false), mRemoteInitFlag(false), mPlayFlag(false), mEnableFlag(false), mSetupBusyFlag(false),
      mSetupCallback(nullptr), mFirstEncodeFlag(false), mForceResumeFlag(false) {
    OSCreateAlarm(&mContinueAlarm);
    OSSetAlarmUserData(&mContinueAlarm, this);

    OSCreateAlarm(&mInvervalAlarm);
    OSSetAlarmUserData(&mInvervalAlarm, this);
}

void RemoteSpeaker::InitParam() {
    ClearParam();

    mPlayFlag = false;
    mEnableFlag = true;

    std::memset(&mEncodeInfo, 0, sizeof(WENCInfo));

    mForceResumeFlag = false;
    mContinueFlag = false;
    mIntervalFlag = false;
    mFirstEncodeFlag = false;
}

void RemoteSpeaker::ClearParam() {
    mPlayFlag = false;
    mEnableFlag = false;

    OSCancelAlarm(&mContinueAlarm);
    mContinueFlag = false;

    OSCancelAlarm(&mInvervalAlarm);
    mIntervalFlag = false;
}

bool RemoteSpeaker::Setup(WPADCallback pCallback) { NW4HBMWarningMessage("RemoteSpeaker::Setup is busy."); }

void RemoteSpeaker::Update(const s16* pRmtSamples) {
    NW4HBMAssert_Line(IsAvailable(), 257);
    if (!IsAvailable()) {
        return;
    }

    ut::AutoInterruptLock lock;
    if (!WPADCanSendStreamData(mChannelIndex)) {
        return;
    }

    bool playFlag = true;
    bool silentFlag = mEnableFlag ? IsAllSampleZero(pRmtSamples) : true;

    if (silentFlag || mForceResumeFlag) {
        playFlag = false;
    }

    u32 wencMode;
    bool firstFlag = false;
    bool lastFlag = false;

    if (mPlayFlag) {
        if (playFlag) {
            wencMode = 1;
        } else {
            wencMode = 1;
            lastFlag = true;
        }
    } else {
        if (playFlag) {
            if (mFirstEncodeFlag) {
                wencMode = 0;
            } else {
                wencMode = 1;
            }

            mFirstEncodeFlag = false;
            firstFlag = true;
        } else {
            wencMode = -1;
        }
    }

    if (wencMode != -1) {
        u8 adpcmBuffer[SAMPLES_PER_ENCODED_PACKET];
        WENCGetEncodeData(&mEncodeInfo, wencMode, pRmtSamples,
                          wencMode == 2 ? SAMPLES_PER_AUDIO_PACKET - 8 : SAMPLES_PER_AUDIO_PACKET, adpcmBuffer);

        s32 result = WPADSendStreamData(mChannelIndex, adpcmBuffer, SAMPLES_PER_ENCODED_PACKET);
        NW4HBMAssertWarningMessage_Line(result == 0, 340, "WPADSendStreamData failed. %d", result);
    }

    if (firstFlag) {
        ut::AutoInterruptLock lock;

        if (!mContinueFlag) {
            OSSetAlarm(&mContinueAlarm, OSSecondsToTicks(CONTINUOUS_PLAY_INTERVAL_MINUTES * 60LL),
                       ContinueAlarmHandler);

            mContinueBeginTime = OSGetTime();
            mContinueFlag = true;
        }

        OSCancelAlarm(&mInvervalAlarm);
        mIntervalFlag = false;
    }

    if (lastFlag) {
        ut::AutoInterruptLock lock;

        mIntervalFlag = true;
        OSCancelAlarm(&mInvervalAlarm);
        OSSetAlarm(&mInvervalAlarm, OSSecondsToTicks(1LL), IntervalAlarmHandler);
    }

    mPlayFlag = playFlag;
}

bool RemoteSpeaker::IsAllSampleZero(const s16* pSample) {
    const u32* buffer = reinterpret_cast<const u32*>(pSample);
    bool zeroFlag = true;

    for (int i = 0; i < SAMPLES_PER_ENCODED_PACKET; i++) {
        if (buffer[i] != 0) {
            zeroFlag = false;
            break;
        }
    }

    return zeroFlag;
}

void RemoteSpeaker::ContinueAlarmHandler(OSAlarm* alarm, OSContext* context) {
    ut::AutoInterruptLock lock;
    RemoteSpeaker* speaker = static_cast<RemoteSpeaker*>(OSGetAlarmUserData(alarm));
    NW4HBMWarningMessage_Line(548, "Playing time of remote speaker(%d) is over 8 minutes.", speaker->mChannelIndex);
    speaker->mForceResumeFlag = true;
    speaker->mContinueFlag = false;
}

void RemoteSpeaker::IntervalAlarmHandler(OSAlarm* alarm, OSContext* context) {
    ut::AutoInterruptLock lock;
    RemoteSpeaker* speaker = static_cast<RemoteSpeaker*>(OSGetAlarmUserData(alarm));

    if (speaker->mIntervalFlag) {
        OSCancelAlarm(&speaker->mContinueAlarm);
        speaker->mForceResumeFlag = false;
        speaker->mContinueFlag = false;
    }

    speaker->mIntervalFlag = false;
}

} // namespace snd
} // namespace nw4hbm
