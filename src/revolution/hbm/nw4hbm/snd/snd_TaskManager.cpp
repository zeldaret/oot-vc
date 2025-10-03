#include "revolution/hbm/nw4hbm/snd/TaskManager.h"

namespace nw4hbm {
namespace snd {
namespace detail {

u8 TaskManager::mTaskArea[0x2000 + 0x44] ATTRIBUTE_ALIGN(32);

TaskManager& TaskManager::GetInstance() {
    static TaskManager instance;
    return instance;
}

TaskManager::TaskManager() :
    mMutex(),
    mCurrentTask(nullptr),
    mTaskList() {
    OSInitMutex(&mMutex);
    mHeapHandle = MEMCreateUnitHeap(mTaskArea, sizeof(mTaskArea), 64);

    // clang-format off
    NW4HBMAssert_Line(MEMCountFreeBlockForUnitHeap( mHeapHandle ) >= TASK_NUM, 55);
    // clang-format on
}

void* TaskManager::Alloc() {
    void* allocBuf;

    NW4HBMAssert_Line(mHeapHandle != MEM_HEAP_INVALID_HANDLE, 69);
    ut::AutoInterruptLock lock;

    allocBuf = MEMAllocFromUnitHeap(mHeapHandle);

    while (allocBuf == nullptr) {
        bool result = ExecuteSingle();
        NW4HBMAssert_Line(result, 76);
        allocBuf = MEMAllocFromUnitHeap(mHeapHandle);
    }

    return allocBuf;
}

u32 TaskManager::GetTaskBufferSize() {
    //! TODO: fake match?
    return *((u32*)mHeapHandle + 0x10);
}

void TaskManager::Free(void* ptr) {
    NW4HBMAssert_Line(mHeapHandle != MEM_HEAP_INVALID_HANDLE, 93);
    ut::AutoInterruptLock lock;
    MEMFreeToUnitHeap(mHeapHandle, ptr);
}

void TaskManager::AppendTask(Task* task, TaskPriority priority) {
    NW4HBMAssertHeaderClampedLValue_Line(priority, PRIORITY_LOW, PRIORITY_MAX, 125);
    ut::AutoInterruptLock lock;
    mTaskList[priority].PushBack(task);
}

Task* TaskManager::PopTask(TaskPriority priority) {
    Task* task;
    ut::AutoInterruptLock lock;

    if (mTaskList[priority].IsEmpty()) {
        return nullptr;
    }

    task = &mTaskList[priority].GetFront();
    mTaskList[priority].PopFront();
    return task;
}

void TaskManager::Execute() {
    while (ExecuteSingle()) {}
}

bool TaskManager::ExecuteSingle() {
    ut::AutoMutexLock lock(mMutex);

    if ((mCurrentTask = PopTask(static_cast<TaskPriority>(PRIORITY_HIGH))) == nullptr) {
        if ((mCurrentTask = PopTask(static_cast<TaskPriority>(PRIORITY_MIDDLE))) == nullptr) {
            if ((mCurrentTask = PopTask(static_cast<TaskPriority>(PRIORITY_LOW))) == nullptr) {
                return false;
            }
        }
    }

    mCurrentTask->Execute();
    Free(mCurrentTask);
    mCurrentTask = nullptr;
    return true;
}

void TaskManager::CancelByTaskId(u32 taskId) {
    ut::AutoInterruptLock lock;

    for (int i = 0; i < PRIORITY_MAX; i++) {
        TaskPriority prio = static_cast<TaskPriority>(i);

        for (TaskList::Iterator it = mTaskList[prio].GetBeginIter(); it != mTaskList[prio].GetEndIter();) {
            TaskList::Iterator currIt = it++;
            if (currIt->GetTaskId() == taskId) {
                mTaskList[prio].Erase(currIt);
                Free(&*currIt);
            }
        }
    }

    if (mCurrentTask && mCurrentTask->GetTaskId() == taskId) {
        mCurrentTask->Cancel();
    }
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
