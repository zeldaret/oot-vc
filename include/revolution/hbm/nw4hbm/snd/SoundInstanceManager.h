#ifndef NW4R_SND_SOUND_INSTANCE_MANAGER_H
#define NW4R_SND_SOUND_INSTANCE_MANAGER_H

#include <new.hpp>

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/InstancePool.h"

#include "revolution/hbm/nw4hbm/ut/inlines.h"
#include "revolution/hbm/nw4hbm/ut/lock.h"

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
        NW4HBMAssertHeaderClampedLRValue_Line(priority, 0, 127, 92);
        ut::AutoInterruptLock lock;
        T* sound;

        void* ptr = mPool.Alloc();

        if (ptr != NULL) {
            sound = new (ptr) T(this);
        } else {
            if (mPriorityList.IsEmpty()) {
                return nullptr;
            }
            sound = &mPriorityList.GetFront();

            if (sound == NULL) {
                return nullptr;
            }

            if (priority < sound->CalcCurrentPlayerPriority()) {
                return nullptr;
            }
            sound->Stop();

            ptr = mPool.Alloc();
            NW4HBMAssertPointerNonnull_Line(ptr, 114);
            sound = new (ptr) T(this);
        }

        InsertPriorityList(sound, priority);
        return sound;
    }

    void Free(T* sound) {
        NW4HBMAssertPointerNonnull_Line(sound, 134);
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
            return nullptr;
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
