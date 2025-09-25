#ifndef NW4R_SND_TASK_H
#define NW4R_SND_TASK_H

#include "revolution/hbm/nw4hbm/ut/LinkList.h"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp" // ut::NonCopyable

namespace nw4hbm {
namespace snd {
namespace detail {

class Task : private ut::NonCopyable {
    friend class TaskManager;

  public:
    Task() : mTaskId(0) {}

    Task(u32 taskId) : mTaskId(taskId) {}

    virtual ~Task() {} // 0x08

    virtual void Execute() = 0; // 0x0C
    virtual void Cancel() = 0; // 0x10

    u32 GetTaskId() const { return mTaskId; }

  public:
    ut::LinkListNode mTaskLink; // 0x04

  private:
    u32 mTaskId; // 0x0C
};

typedef ut::LinkList<Task, offsetof(Task, mTaskLink)> TaskList;

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_TASK_H
