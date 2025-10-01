#ifndef NW4HBM_SND_TASK_THREAD_H
#define NW4HBM_SND_TASK_THREAD_H

#include "revolution/hbm/nw4hbm/snd/types.h"
#include "revolution/hbm/nw4hbm/ut.h"
#include "revolution/os.h"

namespace nw4hbm {
namespace snd {
namespace detail {
class TaskThread {
public:
    TaskThread() :
        mCreateFlag(false) {}

    bool Create(s32 priority);
    BOOL Destroy();

    void SendWakeupMessage();

    void ThreadProc();

    static void* ThreadFunc(void* arg);

    static TaskThread& GetInstance();

    enum {
        MSG_NONE = 0,
        MSG_EXECUTE,
        MSG_DONE,
    };

private:
    static const int MSG_QUEUE_CAPACITY = 8;

    /* 0x00 */ OSThread mThread;
    /* 0x318 */ u64 mThreadStack[THREAD_STACK_SIZE];
    /* 0x2318 */ OSThreadQueue mThreadQueue;

    /* 0x2320 */ OSMessageQueue mMsgQueue;
    /* 0x2340 */ OSMessage mMsgBuffer[MSG_QUEUE_CAPACITY];

    /* 0x2360 */ bool mCreateFlag;
    /* 0x2361 */ u8 mPadding[3];
};
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
