#ifndef NW4HBM_INSTANCEMANAGER_H
#define NW4HBM_INSTANCEMANAGER_H

#include "revolution/hbm/nw4hbm/ut/LinkList.h"

namespace nw4hbm {
namespace snd {
namespace detail {
template <typename T, int Ofs> class InstanceManager {
  public:
    typedef typename ut::LinkList<T, Ofs>::Iterator Iterator;
    void Append(T* obj) {
        NW4HBMAssertPointerNonnull_Line(obj, 67);
        mFreeList.PushBack(obj);
    }

    void Remove(T* obj) {
        NW4HBMAssertPointerNonnull_Line(obj, 84);
        mFreeList.Erase(obj);
    }

    T* Alloc() {
        if (mFreeList.IsEmpty()) {
            return nullptr;
        } else {
            T& obj = mFreeList.GetFront();
            mFreeList.PopFront();
            mActiveList.PushBack(&obj);
            return &obj;
        }
    }
    void Free(T* obj) {
        NW4HBMAssertPointerNonnull_Line(obj, 119);

        if (!mActiveList.IsEmpty()) {
            mActiveList.Erase(obj);
            mFreeList.PushBack(obj);
        }
    }

    Iterator GetBeginIter() { return mActiveList.GetBeginIter(); }
    Iterator GetEndIter() { return mActiveList.GetEndIter(); }

  private:
    ut::LinkList<T, Ofs> mFreeList; // 0x00
    ut::LinkList<T, Ofs> mActiveList; // 0x0C
};
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
