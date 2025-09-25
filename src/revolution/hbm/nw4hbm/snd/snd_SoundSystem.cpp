#include "revolution/hbm/nw4hbm/snd/snd_SoundSystem.hpp"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_SoundSystem.cpp
 */

/*******************************************************************************
 * headers
 */

#include "decomp.h"
#include "macros.h" // STR
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/global.h"
#include "revolution/hbm/nw4hbm/snd/AxManager.h"
#include "revolution/hbm/nw4hbm/snd/snd_AxVoiceManager.hpp"
#include "revolution/hbm/nw4hbm/snd/Channel.h" // ChannelManager
#include "revolution/hbm/nw4hbm/snd/SeqPlayer.h"
#include "revolution/hbm/nw4hbm/snd/SoundThread.h"
#include "revolution/hbm/nw4hbm/snd/TaskManager.h"
#include "revolution/hbm/nw4hbm/snd/TaskThread.h"
#include "revolution/hbm/nw4hbm/snd/snd_VoiceManager.hpp"

#include "revolution/ax/AXVPB.h" // AXGetMaxVoices
#include "revolution/os/OS.h" // OSRegisterVersion
#include "revolution/sc/scapi.h"
#include "revolution/sc/scsystem.h"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace snd {
// .bss
detail::TaskThread SoundSystem::sTaskThread;

// .sbss
namespace {
bool sInitialized;
} // unnamed namespace

int SoundSystem::sMaxVoices;
} // namespace snd
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace snd {

u32 SoundSystem::GetRequiredMemSize(SoundSystemParam const& param) {
    // could have just used align assert? idk
    NW4HBMAssert_Line(param.soundThreadStackSize % 8 == 0, 106);
    NW4HBMAssert_Line(param.dvdThreadStackSize % 8 == 0, 107);

    int maxVoices = AXGetMaxVoices();

    return param.soundThreadStackSize + param.dvdThreadStackSize +
           detail::AxVoiceManager::GetInstance().GetRequiredMemSize(maxVoices) +
           detail::VoiceManager::GetInstance().GetRequiredMemSize(maxVoices) +
           detail::ChannelManager::GetInstance().GetRequiredMemSize(maxVoices);
}

void SoundSystem::InitSoundSystem(SoundSystemParam const& param, void* workMem, u32 workMemSize) {
    bool result; // presumably up here

    NW4HBMAssertAligned_Line(144, workMem, 32);
    NW4HBMAssert_Line(workMemSize >= GetRequiredMemSize(param), 145);

    if (sInitialized) {
        return;
    }

    sInitialized = true;

    // OSRegisterVersion(NW4R_SND_Version_);

    detail::AxManager::GetInstance().Init();

    SCInit();

    SCStatus initStatus;
    do {
        initStatus = (SCStatus)SCCheckStatus();
    } while (initStatus == SC_STATUS_BUSY);

    NW4HBMAssert_Line(initStatus == SC_STATUS_READY, 171);

    SCSoundMode soundMode = (SCSoundMode)SCGetSoundMode();
    switch (soundMode) {
        case SC_SND_MONO:
            detail::AxManager::GetInstance().SetOutputMode(OUTPUT_MODE_MONO);
            break;

        case SC_SND_STEREO:
            detail::AxManager::GetInstance().SetOutputMode(OUTPUT_MODE_STEREO);
            break;

        case SC_SND_SURROUND:
            detail::AxManager::GetInstance().SetOutputMode(OUTPUT_MODE_DPL2);
            break;

        default:
            detail::AxManager::GetInstance().SetOutputMode(OUTPUT_MODE_STEREO);
            break;
    }

    byte_t* ptr = static_cast<byte_t*>(workMem);

    void* dvdThreadStack = ptr;
    ptr += param.dvdThreadStackSize;

    void* soundThreadStack = ptr;
    ptr += param.soundThreadStackSize;

    sMaxVoices = AXGetMaxVoices();

    void* axVoiceWork = ptr;
    ptr += detail::AxVoiceManager::GetInstance().GetRequiredMemSize(sMaxVoices);

    detail::AxVoiceManager::GetInstance().Setup(axVoiceWork,
                                                detail::AxVoiceManager::GetInstance().GetRequiredMemSize(sMaxVoices));

    void* voiceWork = ptr;
    ptr += detail::VoiceManager::GetInstance().GetRequiredMemSize(sMaxVoices);

    detail::VoiceManager::GetInstance().Setup(voiceWork,
                                              detail::VoiceManager::GetInstance().GetRequiredMemSize(sMaxVoices));

    void* channelWork = ptr;
    ptr += detail::ChannelManager::GetInstance().GetRequiredMemSize(sMaxVoices);

    detail::ChannelManager::GetInstance().Setup(channelWork,
                                                detail::ChannelManager::GetInstance().GetRequiredMemSize(sMaxVoices));

    detail::SeqPlayer::InitSeqPlayer();

    result = sTaskThread.Create(param.dvdThreadPriority, dvdThreadStack, param.dvdThreadStackSize);
    NW4HBMAssert_Line(result, 247);

    result = detail::SoundThread::GetInstance().Create(param.soundThreadPriority, soundThreadStack,
                                                       param.soundThreadStackSize);
    NW4HBMAssert_Line(result, 255);

    NW4HBMAssert_Line(ptr <= reinterpret_cast<u8*>(workMem) + workMemSize, 257);
}

void SoundSystem::ShutdownSoundSystem() {
    if (!sInitialized) {
        return;
    }

    detail::SoundThread::GetInstance().Shutdown();

    detail::TaskManager::GetInstance().CancelAllTask();
    sTaskThread.Destroy();

    detail::ChannelManager::GetInstance().Shutdown();
    detail::VoiceManager::GetInstance().Shutdown();
    detail::AxVoiceManager::GetInstance().Shutdown();
    detail::AxManager::GetInstance().Shutdown();

    sInitialized = false;
}

bool SoundSystem::IsInitializedSoundSystem() { return sInitialized; }

// SoundSystem::WaitForResetReady ([R89JEL]:/bin/RVL/Debug/mainD.MAP:14493)
DECOMP_FORCE("SoundSystem::WaitForResetReady is TIME OUT.\n");

} // namespace snd
} // namespace nw4hbm
