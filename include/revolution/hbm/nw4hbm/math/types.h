#ifndef NW4HBM_MATH_TYPES_H
#define NW4HBM_MATH_TYPES_H

#include "revolution/mtx/mtx.h"
#include "revolution/types.h"

namespace nw4hbm {
namespace math {

struct _VEC2 {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
}; // size = 0x08

struct _VEC3 {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
}; // size = 0x0C

struct _MTX34 {
    union {
        struct {
            /* 0x00 */ f32 _00;
            /* 0x04 */ f32 _01;
            /* 0x08 */ f32 _02;
            /* 0x0C */ f32 _03;
            /* 0x10 */ f32 _10;
            /* 0x14 */ f32 _11;
            /* 0x18 */ f32 _12;
            /* 0x1C */ f32 _13;
            /* 0x20 */ f32 _20;
            /* 0x24 */ f32 _21;
            /* 0x28 */ f32 _22;
            /* 0x2C */ f32 _23;
        }; // size = 0x30

        f32 m[3][4]; // size = 0x30
        f32 a[12]; // size = 0x30
        Mtx mtx; // size = 0x30
    /* 0x00 */ };
}; // size = 0x30

struct VEC2 : public _VEC2 {
public:
    VEC2() {}
    VEC2(f32 fx, f32 fy) {
        x = fx;
        y = fy;
    }

    // operators
    operator f32*() { return reinterpret_cast<f32*>(this); }
    operator const f32*() const { return reinterpret_cast<const f32*>(this); }

public:
    /* 0x00 (base) */
}; // size = 0x08

struct VEC3 : public _VEC3 {
public:
    VEC3() {}
    VEC3(f32 fx, f32 fy, f32 fz) {
        x = fx;
        y = fy;
        z = fz;
    }

    // operators
    operator VecPtr() { return reinterpret_cast<VecPtr>(this); }

public:
    /* 0x00 (base) */
}; // size = 0x0C

struct MTX34 : public _MTX34 {
public:
    MTX34() {} // NOTE: must be non-defaulted

    // operators
    operator MtxPtr() { return mtx; }
    operator CMtxPtr() const { return mtx; }

public:
    /* 0x00 (base) */
}; // size = 0x30

inline MTX34* MTX34Copy(MTX34* pOut, const MTX34* p) {
    PSMTXCopy(*(Mtx*)p, *(Mtx*)pOut);
    return pOut;
}

inline MTX34* MTX34Mult(MTX34* pOut, const MTX34* p1, const MTX34* p2) {
    PSMTXConcat(*(Mtx*)p1, *(Mtx*)p2, *(Mtx*)pOut);
    return pOut;
}

inline MTX34* MTX34Identity(MTX34* pOut) {
    PSMTXIdentity(*(Mtx*)pOut);
    return pOut;
}

} // namespace math
} // namespace nw4hbm

#endif
