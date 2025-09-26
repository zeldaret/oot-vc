#include "revolution/hbm/nw4hbm/snd/SoundThread.h"

#include "revolution/hbm/nw4hbm/snd/SeqPlayer.h"
#include "revolution/hbm/nw4hbm/snd/StrmPlayer.h"
#include "revolution/hbm/nw4hbm/snd/WavePlayer.h"
#include "revolution/hbm/nw4hbm/snd/WsdPlayer.h"

#include "revolution/os.h"

#include "revolution/hbm/HBMAssert.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

SoundThread& SoundThread::GetInstance() {
    static SoundThread instance;
    return instance;
}

bool SoundThread::Create(s32 priority) {
    if (mCreateFlag) {
        return true;
    }

    mCreateFlag = true;
    NW4HBMAssertMessage_Line(AxManager::GetInstance().CheckInit(), 68, "not initialized nw4hbm::AxManager.\n");

    OSInitThreadQueue(&mThreadQueue);
    OSInitMutex(&mMutex);

    BOOL result = OSCreateThread(&mThread, SoundThreadFunc, &GetInstance(), mThreadStack + THREAD_STACK_SIZE,
                                  THREAD_STACK_SIZE * 8, priority, 0);

    if (result) {
        OSResumeThread(&mThread);
    }

    return result;
}

BOOL SoundThread::Shutdown() {
    if (!mCreateFlag) {
        return FALSE;
    }

    if (!OSSendMessageAny(&GetInstance().mMsgQueue, MSG_SHUTDOWN, OS_MESSAGE_NO_FLAGS)) {
        return FALSE;
    }

    BOOL result = OSJoinThread(&mThread, nullptr);
    NW4HBMAssert_Line(result != 0, 112);

    mCreateFlag = false;
    return TRUE;
}

void SoundThread::AxCallback() {
    BOOL result = OSSendMessage(&GetInstance().mMsgQueue, reinterpret_cast<OSMessage>(MSG_AX_CALLBACK), 0);

    WavePlayer::detail_UpdateBufferAllPlayers();
    StrmPlayer::UpdateBufferAllPlayers();
}

void* SoundThread::SoundThreadFunc(void* arg) {
    SoundThread* thread = static_cast<SoundThread*>(arg);

    OSInitMessageQueue(&thread->mMsgQueue, thread->mMsgBuffer, MSG_QUEUE_CAPACITY);

    AxManager::GetInstance().RegisterCallback(&thread->mAxCallbackNode, AxCallback);
    thread->SoundThreadProc();
    AxManager::GetInstance().UnregisterCallback(&thread->mAxCallbackNode);

    return NULL;
}

void SoundThread::SoundThreadProc() {
    OSMessage message;

    while (true) {
        OSReceiveMessage(&mMsgQueue, &message, 1);

        if (reinterpret_cast<u32>(message) == MSG_AX_CALLBACK) {
            ut::AutoMutexLock autoMutex(mMutex);

            OSTick start = OSGetTick();
            {
                AxManager::GetInstance().Update();

                SeqPlayer::UpdateAllPlayers();
                WsdPlayer::UpdateAllPlayers();
                StrmPlayer::UpdateAllPlayers();
                WavePlayer::detail_UpdateAllPlayers();

                for (CallbackList::Iterator it = mCallbackList.GetBeginIter(); it != mCallbackList.GetEndIter();) {
                    CallbackList::Iterator currIt = it++;
                    currIt->UpdatePlayer();
                }

                Channel::UpdateAllChannel();
                Util::CalcRandom();

                {
                    ut::AutoInterruptLock lock;
                    AxManager::GetInstance().UpdateAllVoices();
                }
            }
            mProcessTick = OSDiffTick(OSGetTick(), start);

            for (CallbackList::Iterator it = mCallbackList.GetBeginIter(); it != mCallbackList.GetEndIter();) {
                CallbackList::Iterator currIt = it++;
                currIt->EndSoundFrame();
            }
        } else if (reinterpret_cast<u32>(message) == MSG_SHUTDOWN) {
            SeqPlayer::StopAllPlayers();
            WsdPlayer::StopAllPlayers();
            StrmPlayer::StopAllPlayers();
            WavePlayer::detail_StopAllPlayers();

            break;
        }
    }
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
