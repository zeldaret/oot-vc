#ifndef NW4HBM_UT_LOCK_H
#define NW4HBM_UT_LOCK_H

#include "revolution/os/OSMutex.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/hbm/nw4hbm/ut/inlines.h"

namespace nw4hbm {
    namespace ut {
        namespace detail {
            inline void Lock(OSMutex& rMutex) {
                OSLockMutex(&rMutex);
            }
            inline void Unlock(OSMutex& rMutex) {
                OSUnlockMutex(&rMutex);
            }

            template<typename T> class AutoLock : private NonCopyable {
                public:
                    explicit AutoLock(T& rLockObj) : mLockObj(rLockObj) {
                        Lock(rLockObj);
                    }
                    ~AutoLock() {
                        Unlock(mLockObj);
                    }

                private:
                    T&  mLockObj;   // 0x00
            };
        }

        typedef detail::AutoLock<OSMutex>   AutoMutexLock;

        class AutoInterruptLock : private NonCopyable {
            public:
                AutoInterruptLock() : mOldState(OSDisableInterrupts()) {}
                ~AutoInterruptLock() {
                    OSRestoreInterrupts(mOldState);
                }

            private:
                BOOL    mOldState;  // 0x00
        };
    }
}

#endif // NW4HBM_UT_LIST_H
