#ifndef NW4R_SND_TASK_MANAGER_H
#define NW4R_SND_TASK_MANAGER_H

#include "revolution/hbm/nw4hbm/snd/Task.h"

#include "revolution/mem.h"

namespace nw4hbm {
namespace snd {
namespace detail {

class TaskManager {
  public:
    typedef enum TaskPriority {
        PRIORITY_LOW = 0,
        PRIORITY_MIDDLE,
        PRIORITY_HIGH,
        PRIORITY_MAX
    } TaskPriority;

  public:
    static TaskManager& GetInstance();

    void AppendTask(Task* task, TaskPriority priority);
    Task* PopTask(TaskPriority priority);

    void Execute();
    bool ExecuteSingle() NO_INLINE;

    void CancelByTaskId(u32 taskId);

    u32 GetTaskBufferSize();
    void* Alloc();
    void Free(void* ptr);

    static u8 mTaskArea[0x2000 + 0x44];

  private:
    TaskManager();

    OSMutex mMutex; // 0x00
    MEMiHeapHead mHeapHandle; // 0x18

    Task* mCurrentTask; // 0x1C
    TaskList mTaskList[PRIORITY_MAX]; // 0x20
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_TASK_MANAGER_H
