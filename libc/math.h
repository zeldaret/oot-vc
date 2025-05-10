#ifndef _MATH_H_
#define _MATH_H_

#include "fdlibm.h"
#include "intrinsics.h"
#include "macros.h"
#include "revolution/types.h"

#define DONT_INLINE_SQRT

#define M_PI 3.1415926535897932
#define M_SQRT3 1.7320499420166016

extern int __float_nan[];
extern int __float_huge[];
extern int __double_huge[];

#define NAN (*(f32*)__float_nan)
#define INFINITY (*(f32*)__float_huge)
#define HUGE_VAL (*(double*)__double_huge)

// f64 bit-twiddling macros
#define __HI(x) (((s32*)&x)[0])
#define __LO(x) (((s32*)&x)[1])

#define FP_NAN 1
#define FP_INFINITE 2
#define FP_ZERO 3
#define FP_NORMAL 4
#define FP_SUBNORMAL 5

int __fpclassifyd(f64 x);

#define fpclassify(x) __fpclassifyd(x)
#define isnormal(x) (fpclassify(x) == FP_NORMAL)
#define isnan(x) (fpclassify(x) == FP_NAN)
#define isinf(x) (fpclassify(x) == FP_INFINITE)
#define isfinite(x) (fpclassify(x) > FP_INFINITE)

static inline float sinf(float x) { return (float)sin((double)x); }

static inline float cosf(float x) { return (float)cos((double)x); }

static inline double fabs(double x) { return __fabs(x); }

static inline float sqrtf(float x) { return (float)sqrt((double)x); }

static inline float _inv_sqrtf(float x) {
    const float _half = .5f;
    const float _three = 3.0f;

    if (x > 0.0f) {
        float guess = __frsqrte((double)x); /* returns an approximation to  */
        guess = _half * guess * (_three - guess * guess * x); /* now have 8  sig bits         */
        guess = _half * guess * (_three - guess * guess * x); /* now have 16 sig bits         */
        guess = _half * guess * (_three - guess * guess * x); /* now have >24 sig bits        */
        return guess;
    } else if (x) {
        return NAN;
    }
    return INFINITY;
}

#endif
