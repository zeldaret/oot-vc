#ifndef NW4R_SND_SOUND_INSTANCE_MANAGER_H
#define NW4R_SND_SOUND_INSTANCE_MANAGER_H

#include <new.hpp>

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/InstancePool.h"

#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_lock.hpp"

#include "revolution/os/OSMutex.h"

#include "revolution/hbm/HBMAssert.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

template <typename T> class SoundInstanceManager {
  public:
    u32 Create(void* buffer, u32 size) {
        NW4HBMAssertPointerNonnull_Line(buffer, 59);
        return mPool.Create(buffer, size);
    }

    void Destroy(void* buffer, u32 size) {
        NW4HBMAssertPointerNonnull_Line(buffer, 76);
        mPool.Destroy(buffer, size);
    }

    T* Alloc(int priority) {
        ut::AutoInterruptLock lock;
        T* sound;

        void* buffer = mPool.Alloc();

        if (buffer != NULL) {
            sound = new (buffer) T(this);
        } else {
            if (mPriorityList.IsEmpty()) {
                return NULL;
            }
            sound = &mPriorityList.GetFront();

            if (sound == NULL) {
                return NULL;
            }

            if (priority < sound->CalcCurrentPlayerPriority()) {
                return NULL;
            }
            sound->Stop();

            buffer = mPool.Alloc();
            sound = new (buffer) T(this);
        }

        InsertPriorityList(sound, priority);
        return sound;
    }

    void Free(T* sound) {
        ut::AutoInterruptLock lock;

        if (mPriorityList.IsEmpty()) {
            return;
        }

        RemovePriorityList(sound);
        sound->~T();
        mPool.Free(sound);
    }

    u32 GetActiveCount() const { return mPriorityList.GetSize(); }

    u32 GetFreeCount() const { return mPool.Count(); }

    T* GetLowestPrioritySound() {
        if (mPriorityList.IsEmpty()) {
            return NULL;
        }

        return static_cast<T*>(&mPriorityList.GetFront());
    }

    void InsertPriorityList(T* sound, int priority) {
        TPrioList::Iterator it = mPriorityList.GetBeginIter();

        for (; it != mPriorityList.GetEndIter(); it++) {
            if (priority < it->CalcCurrentPlayerPriority()) {
                break;
            }
        }

        mPriorityList.Insert(it, sound);
    }

    void RemovePriorityList(T* sound) { mPriorityList.Erase(sound); }

    void SortPriorityList() {
        TPrioList listsByPrio[T::PRIORITY_MAX + 1];

        while (!mPriorityList.IsEmpty()) {
            T& rSound = mPriorityList.GetFront();
            mPriorityList.PopFront();
            listsByPrio[rSound.CalcCurrentPlayerPriority()].PushBack(&rSound);
        }

        for (int i = 0; i < T::PRIORITY_MAX + 1; i++) {
            while (!listsByPrio[i].IsEmpty()) {
                T& rSound = listsByPrio[i].GetFront();
                listsByPrio[i].PopFront();
                mPriorityList.PushBack(&rSound);
            }
        }
    }

    void UpdatePriority(T* sound, int priority) {
        RemovePriorityList(sound);
        InsertPriorityList(sound, priority);
    }

  private:
    typedef ut::LinkList<T, offsetof(T, mPriorityLink)> TPrioList;

  private:
    MemoryPool<T> mPool; // 0x00
    TPrioList mPriorityList; // 0x04
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_SOUND_INSTANCE_MANAGER_H
