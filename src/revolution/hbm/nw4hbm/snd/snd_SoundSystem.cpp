#include "revolution/hbm/nw4hbm/snd/SoundSystem.h"

#include "revolution/hbm/nw4hbm/snd/SoundThread.h"
#include "revolution/hbm/nw4hbm/snd/TaskThread.h"

#include "revolution/os.h"
#include "revolution/sc.h"

#include "decomp.h"

namespace nw4hbm {
namespace snd {

namespace {
bool sInitialized = false;
}

void SoundSystem::InitSoundSystem(s32 soundThreadPriority, s32 dvdThreadPriority) {
    if (sInitialized) {
        return;
    }

    sInitialized = true;

    detail::AxManager::GetInstance().Init();

    SCInit();

    u32 initStatus;
    do {
        initStatus = SCCheckStatus();
    } while (initStatus == SC_STATUS_BUSY);
    NW4HBMAssert_Line(initStatus == SC_STATUS_OK, 79);

    switch (SCGetSoundMode()) {
        case SC_SND_MONO: {
            detail::AxManager::GetInstance().SetOutputMode(OUTPUT_MODE_MONO);
            break;
        }
        case SC_SND_STEREO: {
            detail::AxManager::GetInstance().SetOutputMode(OUTPUT_MODE_STEREO);
            break;
        }
        case SC_SND_SURROUND: {
            detail::AxManager::GetInstance().SetOutputMode(OUTPUT_MODE_DPL2);
            break;
        }
        default: {
            detail::AxManager::GetInstance().SetOutputMode(OUTPUT_MODE_STEREO);
            break;
        }
    }

    detail::RemoteSpeakerManager::GetInstance().Setup();

    bool result = detail::TaskThread::GetInstance().Create(dvdThreadPriority);
    NW4HBMAssert_Line(result, 126);

    result = detail::SoundThread::GetInstance().Create(soundThreadPriority);
    NW4HBMAssert_Line(result, 130);
}

void SoundSystem::ShutdownSoundSystem() {
    if (!sInitialized) {
        return;
    }

    BOOL result = detail::SoundThread::GetInstance().Shutdown();
    NW4HBMAssert_Line(result, 141);

    result = detail::TaskThread::GetInstance().Destroy();
    NW4HBMAssert_Line(result, 145);

    detail::RemoteSpeakerManager::GetInstance().Shutdown();
    detail::AxManager::GetInstance().Shutdown();

    sInitialized = false;
}

DECOMP_FORCE("SoundSystem::WaitForResetReady is TIME OUT.\n");

} // namespace snd
} // namespace nw4hbm
