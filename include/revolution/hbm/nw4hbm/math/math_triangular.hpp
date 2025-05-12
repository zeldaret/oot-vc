#ifndef RVL_SDK_HBM_NW4HBM_MATH_TRIANGULAR_HPP
#define RVL_SDK_HBM_NW4HBM_MATH_TRIANGULAR_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // f32

/*******************************************************************************
 * macros
 */

#define NW4HBM_DEG_TO_FIDX_MULT_CONSTANT (256.0f / 360.0f)
#define NW4HBM_FIDX_TO_DEG_MULT_CONSTANT (360.0f / 256.0f)

#define NW4HBMDegToFIdx(x) ((x) * NW4HBM_DEG_TO_FIDX_MULT_CONSTANT)
#define NW4HBMFIdxToDeg(x) ((x) * NW4HBM_FIDX_TO_DEG_MULT_CONSTANT)

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace math {
f32 SinFIdx(f32 fidx);
f32 CosFIdx(f32 fidx);

f32 Atan2FIdx(f32 y, f32 x);

inline f32 SinDeg(f32 deg) { return SinFIdx(NW4HBMDegToFIdx(deg)); }

inline f32 CosDeg(f32 deg) { return CosFIdx(NW4HBMDegToFIdx(deg)); }

inline f32 Atan2Deg(f32 y, f32 x) { return NW4HBMFIdxToDeg(Atan2FIdx(y, x)); }
} // namespace math
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_MATH_TRIANGULAR_HPP
