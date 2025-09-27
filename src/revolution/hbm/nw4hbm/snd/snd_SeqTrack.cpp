#include "decomp.h"
#include "revolution/hbm/nw4hbm/snd.h"
#include "revolution/hbm/nw4hbm/ut.h"

namespace nw4hbm {
namespace snd {
namespace detail {

void SeqTrack::SetPlayerTrackNo(int playerTrackNo) {
    NW4HBMAssertHeaderClampedLRValue_Line(playerTrackNo, 0, 16, 37);
    mPlayerTrackNo = playerTrackNo;
}

void SeqTrack::InitParam() {
    mExtVolume = 1.0f;
    mExtPitch = 1.0f;
    mExtPan = 0.0f;
    mExtSurroundPan = 0.0f;
    mPanRange = 1.0f;
    mExtLpfFreq = 0.0f;

    mExtMainSend = 0.0f;
    for (int i = 0; i < AUX_BUS_NUM; i++) {
        mExtFxSend[i] = 0.0f;
    }
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mExtRemoteSend[i] = 0.0f;
        mExtRemoteFxSend[i] = 0.0f;
    }

    mParserTrackParam.baseAddr = nullptr;
    mParserTrackParam.currentAddr = nullptr;

    mParserTrackParam.bankNo = 0;
    mParserTrackParam.prgNo = 0;

    mParserTrackParam.priority = DEFAULT_PRIORITY;
    mParserTrackParam.wait = 0;

    mParserTrackParam.muteFlag = false;
    mParserTrackParam.silenceFlag = false;
    mParserTrackParam.noteFinishWait = false;
    mParserTrackParam.portaFlag = false;

    mParserTrackParam.volume = 127;
    mParserTrackParam.volume2 = 127;

    mParserTrackParam.pan = 0;
    mParserTrackParam.initPan = 0;
    mParserTrackParam.surroundPan = 0;

    mParserTrackParam.pitchBend = 0;

    mParserTrackParam.attack = 0xFF;
    mParserTrackParam.decay = 0xFF;
    mParserTrackParam.sustain = 0xFF;
    mParserTrackParam.release = 0xFF;

    mParserTrackParam.mainSend = 127;
    for (int i = 0; i < AUX_BUS_NUM; i++) {
        mParserTrackParam.fxSend[i] = 0;
    }

    mParserTrackParam.lpfFreq = 64;
    mParserTrackParam.bendRange = DEFAULT_BENDRANGE;

    mParserTrackParam.portaKey = DEFAULT_PORTA_KEY;
    mParserTrackParam.portaTime = 0;

    mParserTrackParam.sweepPitch = 0.0f;
    mParserTrackParam.transpose = 0;

    mParserTrackParam.lfoParam.Init();
    mParserTrackParam.lfoTarget = Channel::LFO_TARGET_PITCH;

    mChannelList = nullptr;

    for (int i = 0; i < VARIABLE_NUM; i++) {
        mTrackVariable[i] = DEFAULT_VARIABLE_VALUE;
    }
}

void SeqTrack::SetSeqData(const void* base, s32 offset) {
    mParserTrackParam.baseAddr = static_cast<const u8*>(base);
    mParserTrackParam.currentAddr = mParserTrackParam.baseAddr + offset;
}

void SeqTrack::Close() {
    ReleaseAllChannel(-1);
    FreeAllChannel();
}

void SeqTrack::UpdateChannelLength() {
    for (Channel* it = mChannelList; it != nullptr; it = it->GetNextTrackChannel()) {

        if (it->GetLength() > 0) {
            it->SetLength(it->GetLength() - 1);
            if (it->GetLength() == 0) {
                it->Release();
            }
        }

        if (!it->IsAutoUpdateSweep()) {
            it->UpdateSweep(1);
        }
    }
}

int SeqTrack::ParseNextTick(bool doNoteOn) {
    if (mParserTrackParam.noteFinishWait) {
        if (mChannelList != nullptr) {
            return 0;
        }

        mParserTrackParam.noteFinishWait = false;
    }

    if (mParserTrackParam.wait > 0 && --mParserTrackParam.wait > 0) {
        return 0;
    }

    if (mParserTrackParam.currentAddr != nullptr) {
        while (mParserTrackParam.wait == 0 && !mParserTrackParam.noteFinishWait) {

            if (Parse(doNoteOn) == PARSE_RESULT_FINISH) {
                return -1;
            }
        }
    }

    return 0;
}

void SeqTrack::StopAllChannel() {
    ut::AutoInterruptLock lock;
    for (Channel* it = mChannelList; it != nullptr; it = it->GetNextTrackChannel()) {

        Channel::FreeChannel(it);
        it->Stop();
    }

    mChannelList = nullptr;
}

void SeqTrack::ReleaseAllChannel(int release) {
    UpdateChannelParam();

    ut::AutoInterruptLock lock;
    AxManager::GetInstance().LockUpdateVoicePriority();

    for (Channel* it = mChannelList; it != nullptr; it = it->GetNextTrackChannel()) {

        if (it->IsActive()) {
            if (release >= 0) {
                NW4HBMAssertHeaderClampedLRValue_Line(release, 0, 127, 270);
                it->SetRelease(static_cast<u8>(release));
            }

            it->Release();
        }
    }

    AxManager::GetInstance().UnlockUpdateVoicePriority();
}

void SeqTrack::PauseAllChannel(bool flag) {
    ut::AutoInterruptLock lock;

    for (Channel* it = mChannelList; it != nullptr; it = it->GetNextTrackChannel()) {

        if (it->IsActive() && flag != it->IsPause()) {
            it->Pause(flag);
        }
    }
}

void SeqTrack::AddChannel(Channel* channel) {
    ut::AutoInterruptLock lock;
    channel->SetNextTrackChannel(mChannelList);
    mChannelList = channel;
}

void SeqTrack::UpdateChannelParam() {
    f32 volume = 1.0f;
    f32 parserVolume = mParserTrackParam.volume / 127.0f;
    f32 parserVolume2 = mParserTrackParam.volume2 / 127.0f;
    f32 parserMainVolume = mSeqPlayer->GetParserPlayerParam().volume / 127.0f;

    volume *= (parserVolume * parserVolume);
    volume *= (parserVolume2 * parserVolume2);
    volume *= (parserMainVolume * parserMainVolume);
    volume *= mExtVolume;
    volume *= mSeqPlayer->GetVolume();

    f32 pitch = (mParserTrackParam.pitchBend / 128.0f) * mParserTrackParam.bendRange;

    f32 pitchRatio = 1.0f;
    pitchRatio *= mSeqPlayer->GetPitch();
    pitchRatio *= mExtPitch;

    f32 pan = 0.0f;
    pan += ut::Clamp(mParserTrackParam.pan / 63.0f, -1.0f, 1.0f);
    pan *= mPanRange;
    pan *= mSeqPlayer->GetPanRange();
    pan += mExtPan;
    pan += mSeqPlayer->GetPan();

    f32 surroundPan = 0.0f;
    surroundPan += ut::Clamp(mParserTrackParam.surroundPan / 63.0f, 0.0f, 2.0f);
    surroundPan += mExtSurroundPan;
    surroundPan += mSeqPlayer->GetSurroundPan();

    f32 pan2 = 0.0f;
    pan2 += mSeqPlayer->GetPan2();

    f32 surroundPan2 = 0.0f;
    surroundPan2 += mSeqPlayer->GetSurroundPan2();

    f32 lpfFreq = 0.0f;
    lpfFreq += (mParserTrackParam.lpfFreq - 64) / 64.0f;
    lpfFreq += mExtLpfFreq;
    lpfFreq += mSeqPlayer->GetLpfFreq();

    f32 mainSend = 0.0f;
    mainSend += (mParserTrackParam.mainSend / 127.0f) - 1.0f;
    mainSend += mExtMainSend;
    mainSend += mSeqPlayer->GetMainSend();

    f32 fxSend[AUX_BUS_NUM];
    for (int i = 0; i < AUX_BUS_NUM; i++) {
        fxSend[i] = 0.0f;
        fxSend[i] += mParserTrackParam.fxSend[i] / 127.0f;
        fxSend[i] += mExtFxSend[i];
        fxSend[i] += mSeqPlayer->GetFxSend(static_cast<AuxBus>(i));
    }

    f32 remoteSend[WPAD_MAX_CONTROLLERS];
    f32 remoteFxSend[WPAD_MAX_CONTROLLERS];
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        remoteSend[i] = 0.0f;
        remoteSend[i] += mSeqPlayer->GetRemoteSend(i);

        remoteFxSend[i] = 0.0f;
        remoteFxSend[i] += mSeqPlayer->GetRemoteFxSend(i);
    }

    ut::AutoInterruptLock lock;
    for (Channel* it = mChannelList; it != nullptr; it = it->GetNextTrackChannel()) {

        it->SetUserVolume(volume);
        it->SetUserPitch(pitch);
        it->SetUserPitchRatio(pitchRatio);
        it->SetUserPan(pan);
        it->SetUserSurroundPan(surroundPan);
        it->SetUserPan2(pan2);
        it->SetUserSurroundPan2(surroundPan2);
        it->SetUserLpfFreq(lpfFreq);
        it->SetOutputLine(mSeqPlayer->GetOutputLine());
        it->SetMainOutVolume(mSeqPlayer->GetMainOutVolume());
        it->SetMainSend(mainSend);

        for (int i = 0; i < AUX_BUS_NUM; i++) {
            it->SetFxSend(static_cast<AuxBus>(i), fxSend[i]);
        }

        for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
            it->SetRemoteOutVolume(i, mSeqPlayer->GetRemoteOutVolume(i));
            it->SetRemoteSend(i, remoteSend[i]);
            it->SetRemoteFxSend(i, remoteFxSend[i]);
        }

        it->SetLfoParam(mParserTrackParam.lfoParam);
        it->SetLfoTarget(static_cast<Channel::LfoTarget>(mParserTrackParam.lfoTarget));
    }
}

void SeqTrack::FreeAllChannel() {
    ut::AutoInterruptLock lock;
    for (Channel* it = mChannelList; it != nullptr; it = it->GetNextTrackChannel()) {

        Channel::FreeChannel(it);
    }

    mChannelList = nullptr;
}

void SeqTrack::ChannelCallbackFunc(Channel* dropChannel, Channel::ChannelCallbackStatus status, u32 arg) {
    SeqTrack* track = reinterpret_cast<SeqTrack*>(arg);
    NW4HBMAssertPointerNonnull_Line(dropChannel, 521);
    NW4HBMAssertPointerNonnull_Line(track, 522);

    switch (status) {
        case Channel::CALLBACK_STATUS_STOPPED:
        case Channel::CALLBACK_STATUS_FINISH: {
            Channel::FreeChannel(dropChannel);
            break;
        }
    }

    if (track->mSeqPlayer != nullptr) {
        track->mSeqPlayer->ChannelCallback(dropChannel);
    }

    ut::AutoInterruptLock lock;
    if (track->mChannelList == dropChannel) {
        track->mChannelList = dropChannel->GetNextTrackChannel();
        return;
    }

    Channel* channel = track->mChannelList;
    NW4HBMAssertPointerNonnull_Line(channel, 544);
    while (channel->GetNextTrackChannel() != nullptr) {
        if (channel->GetNextTrackChannel() == dropChannel) {
            channel->SetNextTrackChannel(dropChannel->GetNextTrackChannel());
            return;
        }

        channel = channel->GetNextTrackChannel();
    }

    NW4HBMAssert_Line(false, 553);
}
void SeqTrack::SetMute(SeqMute mute) {
    ut::AutoInterruptLock lock;
    switch (mute) {
        case MUTE_OFF: {
            mParserTrackParam.muteFlag = false;
            break;
        }
        case MUTE_STOP: {
            StopAllChannel();
            mParserTrackParam.muteFlag = true;
            break;
        }
        case MUTE_RELEASE: {
            ReleaseAllChannel(-1);
            FreeAllChannel();
            mParserTrackParam.muteFlag = true;
            break;
        }
        case MUTE_NO_STOP: {
            mParserTrackParam.muteFlag = true;
            break;
        }
    }
}

void SeqTrack::SetSilence(bool silence, int fadeTime) {
    mParserTrackParam.silenceFlag = silence;

    ut::AutoInterruptLock lock;
    for (Channel* it = mChannelList; it != nullptr; it = it->GetNextTrackChannel()) {
        it->SetSilence(silence, (fadeTime + 2) / 3);
    }
}

void SeqTrack::SetVolume(f32 volume) {
    ut::AutoInterruptLock lock;
    mExtVolume = volume;
}

void SeqTrack::SetPitch(f32 pitch) {
    NW4HBMAssert_Line(pitch >= 0.0f, 603);
    ut::AutoInterruptLock lock;
    mExtPitch = pitch;
}

void SeqTrack::SetPan(f32 param1) {
    ut::AutoInterruptLock lock;
    mExtPan = param1;
}

void SeqTrack::SetSurroundPan(f32 param1) {
    ut::AutoInterruptLock lock;
    mExtSurroundPan = param1;
}

void SeqTrack::SetPanRange(f32 param1) {
    ut::AutoInterruptLock lock;
    mPanRange = param1;
}

void SeqTrack::SetLpfFreq(f32 param1) {
    ut::AutoInterruptLock lock;
    mExtLpfFreq = param1;
}

void SeqTrack::SetModDepth(f32 param1) {
    ut::AutoInterruptLock lock;
    mParserTrackParam.lfoParam.depth = param1;
}

void SeqTrack::SetModSpeed(f32 param1) {
    ut::AutoInterruptLock lock;
    mParserTrackParam.lfoParam.speed = param1;
}

void SeqTrack::SetMainSend(f32 param1) {
    ut::AutoInterruptLock lock;
    mExtMainSend = param1;
}

void SeqTrack::SetFxSend(AuxBus bus, f32 param2) {
    NW4HBMAssertHeaderClampedLValue_Line(bus, AUX_A, AUX_BUS_NUM, 661);
    ut::AutoInterruptLock lock;
    mExtFxSend[bus] = param2;
}

void SeqTrack::SetRemoteSend(WPADChannel remoteIndex, f32 param2) {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 675);
    ut::AutoInterruptLock lock;
    mExtRemoteSend[remoteIndex] = param2;
}

void SeqTrack::SetRemoteFxSend(WPADChannel remoteIndex, f32 param2) {
    NW4HBMAssertHeaderClampedLValue_Line(remoteIndex, 0, 4, 688);
    ut::AutoInterruptLock lock;
    mExtRemoteFxSend[remoteIndex] = param2;
}

DECOMP_FORCE(NW4HBMAssertHeaderClampedLValue_String(varNo));

vs16* SeqTrack::GetVariablePtr(int varNo) {
    NW4HBMAssertHeaderClampedLRValue_Line(varNo, 0, 16, 747);

    if (varNo < VARIABLE_NUM) {
        return &mTrackVariable[varNo];
    }

    return nullptr;
}

Channel* SeqTrack::NoteOn(int key, int velocity, s32 portatime, bool tie) {
    NW4HBMAssertHeaderClampedLRValue_Line(key, 0, 127, 778);
    NW4HBMAssertHeaderClampedLRValue_Line(velocity, 0, 127, 779);

    SeqPlayer* seqPlayer = GetSeqPlayer();
    Channel* channel = nullptr;

    if (tie) {
        channel = GetLastChannel();
        if (channel != nullptr) {
            channel->SetKey(static_cast<u8>(key));

            f32 initVolume = velocity / 127.0f;
            channel->SetInitVolume(initVolume * initVolume);
        }
    }

    if (channel == nullptr) {
        NoteOnInfo info = {
            mParserTrackParam.prgNo, // prgNo
            key, // key
            velocity, // velocity
            tie ? -1 : portatime, // length
            mParserTrackParam.initPan, // initPan
            seqPlayer->GetParserPlayerParam().priority + // priority
                GetParserTrackParam().priority,
            mSeqPlayer->GetVoiceOutCount(), // voiceOuCount
            ChannelCallbackFunc, // channelCallback
            reinterpret_cast<u32>(this) // channelCallbackData
        };

        channel = mSeqPlayer->NoteOn(mParserTrackParam.bankNo, info);
        if (channel == nullptr) {
            return nullptr;
        }

        AddChannel(channel);
    }

    if (mParserTrackParam.attack != 0xFF) {
        channel->SetAttack(mParserTrackParam.attack);
    }
    if (mParserTrackParam.decay != 0xFF) {
        channel->SetDecay(mParserTrackParam.decay);
    }
    if (mParserTrackParam.sustain != 0xFF) {
        channel->SetSustain(mParserTrackParam.sustain);
    }
    if (mParserTrackParam.release != 0xFF) {
        channel->SetRelease(mParserTrackParam.release);
    }

    f32 sweepPitch = mParserTrackParam.sweepPitch;
    if (mParserTrackParam.portaFlag) {
        sweepPitch += mParserTrackParam.portaKey - key;
    }

    if (mParserTrackParam.portaTime == 0) {
        NW4HBMAssertWarningMessage_Line(portatime != 0, 856, "portatime zero is invalid.");
        channel->SetSweepParam(sweepPitch, portatime, false);
    } else {
        int time = mParserTrackParam.portaTime;
        time *= time;
        time *= sweepPitch >= 0.0f ? sweepPitch : -sweepPitch;
        time = static_cast<u32>(time >> 5);
        time = static_cast<u32>(time << 2) + time;
        channel->SetSweepParam(sweepPitch, time, true);
    }

    mParserTrackParam.portaKey = key;
    channel->SetSilence(mParserTrackParam.silenceFlag != 0, 0);

    return channel;
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
