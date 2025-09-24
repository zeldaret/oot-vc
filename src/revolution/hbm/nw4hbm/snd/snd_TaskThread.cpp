#include "revolution/hbm/nw4hbm/snd/snd_TaskThread.hpp"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_TaskThread.cpp
 */

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/snd_TaskManager.hpp"

#include "revolution/os/OSThread.h"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {

TaskThread::TaskThread() : mStackEnd(nullptr), mFinishFlag(false), mCreateFlag(false) {}

TaskThread::~TaskThread() {
    if (mCreateFlag) {
        Destroy();
    }
}

bool TaskThread::Create(s32 priority, void* stack, u32 stackSize) {
    NW4HBMAssertPointerNonnull_Line(stack, 59);
    NW4HBMAssertAligned_Line(60, stack, 4);

    if (mCreateFlag) {
        Destroy();
    }

    BOOL result = OSCreateThread(&mThread, &ThreadFunc, this, static_cast<byte_t*>(stack) + stackSize, stackSize,
                                 priority, OS_THREAD_NO_FLAGS);
    if (!result) {
        return false;
    }

    mStackEnd = static_cast<byte4_t*>(stack);
    mFinishFlag = false;
    mCreateFlag = true;

    OSResumeThread(&mThread);

    return true;
}

void TaskThread::Destroy() {
    if (!mCreateFlag) {
        return;
    }

    mFinishFlag = true;
    TaskManager::GetInstance().CancelWaitTask();

    BOOL result = OSJoinThread(&mThread, nullptr);
    NW4HBMAssert_Line(result, 105);

    mCreateFlag = false;
}

void* TaskThread::ThreadFunc(void* arg) {
    TaskThread* taskThread = static_cast<TaskThread*>(arg);

    taskThread->ThreadProc();

    return nullptr;
}

void TaskThread::ThreadProc() {
    while (!mFinishFlag) // TODO: implies volatile?
    {
        TaskManager::GetInstance().WaitTask();

        if (mFinishFlag) {
            break;
        }

        TaskManager::GetInstance().ExecuteTask();

        NW4HBMAssert_Line(*mStackEnd == OS_THREAD_STACK_MAGIC, 160);
    }
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
