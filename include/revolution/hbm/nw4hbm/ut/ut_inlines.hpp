#ifndef RVL_SDK_HBM_NW4HBM_UT_INLINES_HPP
#define RVL_SDK_HBM_NW4HBM_UT_INLINES_HPP

#include "revolution/hbm/nw4hbm/db/assert.hpp"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace ut {
namespace {
class NonCopyable {
    // methods
  public:
    // cdtors
    NonCopyable() {}
    ~NonCopyable() {}

    // deleted methods
  private:
    NonCopyable(const NonCopyable&);
    NonCopyable& operator=(const NonCopyable&);
}; // size 0x01 (0x00 for inheritance)

template <typename T> inline T Min(T a, T b) { return a > b ? b : a; }

template <typename T> inline T Max(T a, T b) { return a < b ? b : a; }

template <typename T> inline T Clamp(T value, T min, T max) { return value > max ? max : (value < min ? min : value); }

template <typename T> inline T Abs(T x) {
    // Static cast needed to break abs optimization
    return x < 0 ? static_cast<T>(-x) : static_cast<T>(x);
}

template <> f32 inline Abs(register f32 x) {
    register f32 ax;

#ifdef __MWERKS__ // clang-format off
    asm {
        fabs ax, x
    }
#endif // clang-format on

    return ax;
}

inline u32 GetIntPtr(const void* pPtr) { return reinterpret_cast<u32>(pPtr); }

template <typename T> inline const void* AddOffsetToPtr(const void* pBase, T offset) {
    return reinterpret_cast<const void*>(GetIntPtr(pBase) + offset);
}
template <typename T> inline void* AddOffsetToPtr(void* pBase, T offset) {
    return reinterpret_cast<void*>(GetIntPtr(pBase) + offset);
}

inline s32 GetOffsetFromPtr(const void* pStart, const void* pEnd) {
    return static_cast<s32>(GetIntPtr(pEnd) - GetIntPtr(pStart));
}

inline int ComparePtr(const void* pPtr1, const void* pPtr2) {
    return static_cast<int>(GetIntPtr(pPtr1) - GetIntPtr(pPtr2));
}

template <typename T> inline T RoundUp(T t, unsigned int alignment) { return (alignment + t - 1) & ~(alignment - 1); }

template <typename T> inline void* RoundUp(T* pPtr, unsigned int alignment) {
    u32 value = reinterpret_cast<u32>(pPtr);
    u32 rounded = (alignment + value - 1) & ~(alignment - 1);
    return reinterpret_cast<void*>(rounded);
}

template <typename T> inline T RoundDown(T t, unsigned int alignment) { return t & ~(alignment - 1); }

template <typename T> inline void* RoundDown(T* pPtr, unsigned int alignment) {
    u32 value = reinterpret_cast<u32>(pPtr);
    u32 rounded = value & ~(alignment - 1);
    return reinterpret_cast<void*>(rounded);
}

} // namespace
} // namespace ut
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_UT_INLINES_HPP
