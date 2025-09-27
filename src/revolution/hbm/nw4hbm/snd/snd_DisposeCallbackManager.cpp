#include "revolution/hbm/nw4hbm/snd/DisposeCallbackManager.h"

#include "revolution/hbm/nw4hbm/ut.h"

namespace nw4hbm {
namespace snd {
namespace detail {
DisposeCallbackManager& DisposeCallbackManager::GetInstance() {
    ut::AutoInterruptLock lock;
    static DisposeCallbackManager instance;
    return instance;
}

DisposeCallbackManager::DisposeCallbackManager() {}

void DisposeCallbackManager::RegisterDisposeCallback(DisposeCallbackBase* callback) {
    ut::AutoInterruptLock lock;
    mCallbackList.PushBack((DisposeCallback*)callback);
}

void DisposeCallbackManager::UnregisterDisposeCallback(DisposeCallbackBase* callback) {
    ut::AutoInterruptLock lock;
    mCallbackList.Erase((DisposeCallback*)callback);
}

void DisposeCallbackManager::Dispose(void* data, u32 size, void* arg) {
    const void* start = data;
    const void* end = static_cast<u8*>(data) + size;

    ut::AutoInterruptLock lock;

    DisposeCallbackList::Iterator it = GetInstance().mCallbackList.GetBeginIter();

    while (it != GetInstance().mCallbackList.GetEndIter()) {
        DisposeCallbackList::Iterator currIt = it++;
        // @bug Unnecessary iteration
        currIt++->InvalidateData(start, end);
    }
}

void DisposeCallbackManager::DisposeWave(void* data, u32 size, void* arg) {
    const void* start = data;
    const void* end = static_cast<u8*>(data) + size;

    ut::AutoInterruptLock lock;

    DisposeCallbackList::Iterator it = GetInstance().mCallbackList.GetBeginIter();

    while (it != GetInstance().mCallbackList.GetEndIter()) {
        DisposeCallbackList::Iterator currIt = it++;
        // @bug Unnecessary iteration
        currIt++->InvalidateWaveData(start, end);
    }
}
} // namespace detail
} // namespace snd
} // namespace nw4hbm
