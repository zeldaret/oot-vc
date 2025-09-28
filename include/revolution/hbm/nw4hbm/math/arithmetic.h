#ifndef RVL_SDK_HBM_NW4HBM_MATH_ARITHMETIC_HPP
#define RVL_SDK_HBM_NW4HBM_MATH_ARITHMETIC_HPP

#include "revolution/os/OSFastCast.h"
#include "revolution/types.h"

namespace nw4hbm {
namespace math {
inline f32 FSelect(register f32 cond, register f32 ifPos, register f32 ifNeg) {
    register f32 ret;

#ifdef __clang__
    asm("fsel %0, %1, %2, %3" : "=f"(ret) : "f"(cond), "f"(ifPos), "f"(ifNeg));
#else
    asm { fsel ret, cond, ifPos, ifNeg }
    ;
#endif // __clang__

    return ret;
}

inline f32 FAbs(f32 x) {
    f32 ret;

    asm("fabs %0, %1" : "=f"(ret) : "f"(x));

    return ret;
}

inline u16 F32ToU16(f32 x) {
    u16 rval;

    OSf32tou16(&x, &rval);

    return rval;
}

inline f32 U16ToF32(u16 x) {
    f32 rval;

    OSu16tof32(&x, &rval);

    return rval;
}
} // namespace math
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_MATH_ARITHMETIC_HPP
