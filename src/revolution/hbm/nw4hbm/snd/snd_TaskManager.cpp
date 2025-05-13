#include "revolution/hbm/snd.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

TaskManager& TaskManager::GetInstance() {
    static TaskManager instance;
    return instance;
}

TaskManager::TaskManager() : mCurrentTask(nullptr), mCancelWaitTaskFlag(false) {
    OSInitThreadQueue(&mAppendThreadQueue);
    OSInitThreadQueue(&mDoneThreadQueue);
}

void TaskManager::AppendTask(Task* pTask, TaskPriority priority) {
    ut::AutoInterruptLock lock;

    pTask->mBusyFlag = true;
    mTaskList[priority].PushBack(pTask);

    OSWakeupThread(&mAppendThreadQueue);
}

Task* TaskManager::GetNextTask(TaskPriority priority, bool remove) {
    ut::AutoInterruptLock lock;

    if (mTaskList[priority].IsEmpty()) {
        return nullptr;
    }

    Task& rTask = mTaskList[priority].GetFront();

    if (remove) {
        mTaskList[priority].PopFront();
    }

    return &rTask;
}

Task* TaskManager::PopTask() {
    ut::AutoInterruptLock lock;
    Task* pTask;

    pTask = GetNextTask(PRIORITY_HIGH, true);
    if (pTask != nullptr) {
        return pTask;
    }

    pTask = GetNextTask(PRIORITY_MIDDLE, true);
    if (pTask != nullptr) {
        return pTask;
    }

    pTask = GetNextTask(PRIORITY_LOW, true);
    if (pTask != nullptr) {
        return pTask;
    }

    return nullptr;
}

Task* TaskManager::GetNextTask() {
    ut::AutoInterruptLock lock;
    Task* pTask;

    pTask = GetNextTask(PRIORITY_HIGH, false);
    if (pTask != nullptr) {
        return pTask;
    }

    pTask = GetNextTask(PRIORITY_MIDDLE, false);
    if (pTask != nullptr) {
        return pTask;
    }

    pTask = GetNextTask(PRIORITY_LOW, false);
    if (pTask != nullptr) {
        return pTask;
    }

    return nullptr;
}

Task* TaskManager::ExecuteTask() {
    Task* pTask = PopTask();
    if (pTask == nullptr) {
        return nullptr;
    }

    mCurrentTask = pTask;
    {
        pTask->mBusyFlag = false;
        pTask->Execute();
    }
    mCurrentTask = nullptr;

    OSWakeupThread(&mDoneThreadQueue);
    return pTask;
}

void TaskManager::CancelTask(Task* pTask) {
    ut::AutoInterruptLock lock;

    if (pTask == mCurrentTask) {
        pTask->OnCancel();

        while (pTask == mCurrentTask) {
            OSSleepThread(&mDoneThreadQueue);
        }
    } else {
        for (int i = 0; i < PRIORITY_MAX; i++) {
            TaskPriority priority = static_cast<TaskPriority>(i);
            TaskList& rList = mTaskList[priority];

            NW4R_UT_LINKLIST_FOREACH_SAFE(it, rList, {
                if (&*it == pTask) {
                    rList.Erase(it);

                    it->mBusyFlag = false;
                    it->Cancel();
                    break;
                }
            })
        }
    }
}

void TaskManager::CancelAllTask() {
    ut::AutoInterruptLock lock;

    for (int i = 0; i < PRIORITY_MAX; i++) {
        TaskPriority priority = static_cast<TaskPriority>(i);
        TaskList& rList = mTaskList[priority];

        while (!rList.IsEmpty()) {
            Task& rTask = rList.GetBack();
            rList.PopBack();

            rTask.mBusyFlag = false;
            rTask.Cancel();
        }
    }

    if (mCurrentTask != nullptr) {
        mCurrentTask->OnCancel();

        while (mCurrentTask != nullptr) {
            OSSleepThread(&mDoneThreadQueue);
        }
    }
}

void TaskManager::WaitTask() {
    ut::AutoInterruptLock lock;

    mCancelWaitTaskFlag = false;

    while (GetNextTask() == nullptr && !mCancelWaitTaskFlag) {
        OSSleepThread(&mAppendThreadQueue);
    }
}

void TaskManager::CancelWaitTask() {
    ut::AutoInterruptLock lock;

    mCancelWaitTaskFlag = true;
    OSWakeupThread(&mAppendThreadQueue);
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
