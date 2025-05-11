#ifndef RVL_SDK_HBM_NW4HBM_LYT_TYPES_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_TYPES_HPP

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace lyt {
namespace detail {
template <typename T> const T* ConvertOffsToPtr(const void* baseAddress, unsigned offset) {
    return reinterpret_cast<const T*>(reinterpret_cast<unsigned>(baseAddress) + offset);
}

template <typename T> T* ConvertOffsToPtr(void* baseAddress, unsigned offset) {
    return reinterpret_cast<T*>(reinterpret_cast<unsigned>(baseAddress) + offset);
}

template <typename T> void SetBit(T* pBits, int pos, bool val) {
    T mask = static_cast<T>(~(1 << pos));

    *pBits &= mask;
    *pBits |= (val ? 1 : 0) << pos;
}

template <typename T> bool TestBit(T test, int pos) {
    T mask = static_cast<T>(1 << pos);

    return (test & mask) != 0;
}

template <typename T> T GetBits(T bits, int pos, int len) {
    T mask = static_cast<T>(~(-1 << len));

    return static_cast<T>((bits >> pos) & mask);
}
} // namespace detail
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_TYPES_HPP
