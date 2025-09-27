#include "revolution/hbm/nw4hbm/snd/RemoteSpeakerManager.h"

#include "decomp.h"
#include "revolution/ax.h"
#include "revolution/hbm/HBMAssert.hpp"
#include "revolution/os.h"

namespace nw4hbm {
namespace snd {
namespace detail {

RemoteSpeakerManager& RemoteSpeakerManager::GetInstance() {
    static RemoteSpeakerManager instance;
    return instance;
}

RemoteSpeakerManager::RemoteSpeakerManager() : mInitialized(false) {
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mSpeaker[i].SetChannelIndex(i);
    }
}

RemoteSpeaker& RemoteSpeakerManager::GetRemoteSpeaker(int channelIndex) {
    NW4HBMAssertHeaderClampedLValue(channelIndex, 0, 4);
    return mSpeaker[channelIndex];
}

void RemoteSpeakerManager::Setup() {
    if (mInitialized) {
        return;
    }

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mSpeaker[i].InitParam();
    }

    OSCreateAlarm(&mRemoteSpeakerAlarm);
    OSSetPeriodicAlarm(&mRemoteSpeakerAlarm, OSGetTime(), OSNanosecondsToTicks(SPEAKER_ALARM_PERIOD_NSEC),
                       RemoteSpeakerAlarmProc);

    mInitialized = true;
}

void RemoteSpeakerManager::Shutdown() {
    if (!mInitialized) {
        return;
    }

    OSCancelAlarm(&mRemoteSpeakerAlarm);
    mInitialized = false;
}

void RemoteSpeakerManager::RemoteSpeakerAlarmProc(OSAlarm* alarm, OSContext* context) {
    RemoteSpeakerManager& manager = GetInstance();
    NW4HBMAssert_Line(&manager.mRemoteSpeakerAlarm == alarm, 141);

    s16 samples[RemoteSpeaker::SAMPLES_PER_AUDIO_PACKET];
    if (AXRmtGetSamplesLeft() < RemoteSpeaker::SAMPLES_PER_AUDIO_PACKET) {
        return;
    }

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        if (manager.mSpeaker[i].IsAvailable()) {
            s32 sampleSize = AXRmtGetSamples(i, samples, RemoteSpeaker::SAMPLES_PER_AUDIO_PACKET);
            NW4HBMAssertWarningMessage_Line(sampleSize == RemoteSpeaker::SAMPLES_PER_AUDIO_PACKET, 157,
                                            "wrong remote sample size");
            manager.mSpeaker[i].Update(samples);
        }
    }

    AXRmtAdvancePtr(RemoteSpeaker::SAMPLES_PER_AUDIO_PACKET);
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
