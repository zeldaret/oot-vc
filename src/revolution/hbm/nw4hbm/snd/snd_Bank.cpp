#include "revolution/hbm/nw4hbm/snd/Bank.h"

#include "revolution/hbm/nw4hbm/ut.h"

namespace nw4hbm {
namespace snd {
namespace detail {

Bank::Bank(const void* bankData) :
    mBankReader(bankData),
    mWaveDataAddress(nullptr) {}

Bank::~Bank() {}

Channel* Bank::NoteOn(const NoteOnInfo& noteOnInfo) const {
    InstInfo instInfo;
    if (!mBankReader.ReadInstInfo(&instInfo, noteOnInfo.prgNo, noteOnInfo.key, noteOnInfo.velocity)) {
        return nullptr;
    }

    WaveData waveData;
    if (!mBankReader.ReadWaveParam(&waveData, instInfo.waveIndex, mWaveDataAddress)) {
        return nullptr;
    }

    Channel* channel =
        Channel::AllocChannel(ut::Min<int>(waveData.numChannels, CHANNEL_MAX), noteOnInfo.voiceOutCount,
                              noteOnInfo.priority, noteOnInfo.channelCallback, noteOnInfo.channelCallbackData);

    if (channel == nullptr) {
        return nullptr;
    }

    channel->SetKey(noteOnInfo.key);
    channel->SetOriginalKey(instInfo.originalKey);

    f32 initVolume = noteOnInfo.velocity / 127.0f;
    initVolume *= initVolume;
    initVolume *= instInfo.volume / 127.0f;
    channel->SetInitVolume(initVolume);

    channel->SetTune(instInfo.tune);
    channel->SetAttack(instInfo.attack);
    channel->SetDecay(instInfo.decay);
    channel->SetSustain(instInfo.sustain);
    channel->SetRelease(instInfo.release);

    f32 initPan = (instInfo.pan - 64) / 63.0f;
    initPan += noteOnInfo.initPan / 63.0f;
    channel->SetInitPan(initPan);

    channel->SetInitSurroundPan(0.0f);
    channel->Start(waveData, noteOnInfo.length);

    return channel;
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
