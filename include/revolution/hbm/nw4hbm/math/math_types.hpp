#ifndef RVL_SDK_HBM_NW4HBM_MATH_TYPES_HPP
#define RVL_SDK_HBM_NW4HBM_MATH_TYPES_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/mtx/mtx.h"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace math {
//
// Base types
//

struct _VEC2 {
    f32 x; // size 0x04, offset 0x00
    f32 y; // size 0x04, offset 0x04
}; // size 0x08

struct _VEC3 {
    f32 x; // size 0x04, offset 0x00
    f32 y; // size 0x04, offset 0x04
    f32 z; // size 0x04, offset 0x08
}; // size 0x0c

struct _MTX34 {
    union /* explicitly untagged */
    {
        struct /* explicitly untagged */
        {
            f32 _00; // size 0x04, offset 0x00
            f32 _01; // size 0x04, offset 0x04
            f32 _02; // size 0x04, offset 0x08
            f32 _03; // size 0x04, offset 0x0c
            f32 _10; // size 0x04, offset 0x10
            f32 _11; // size 0x04, offset 0x14
            f32 _12; // size 0x04, offset 0x18
            f32 _13; // size 0x04, offset 0x1c
            f32 _20; // size 0x04, offset 0x20
            f32 _21; // size 0x04, offset 0x24
            f32 _22; // size 0x04, offset 0x28
            f32 _23; // size 0x04, offset 0x2c
        }; // size 0x30

        f32 m[3][4]; // size 0x30
        f32 a[12]; // size 0x30
        Mtx mtx; // size 0x30
    }; // size 0x30, offset 0x00
}; // size 0x30

//
// Derived types
//

struct VEC2 : public _VEC2 {
    // methods
  public:
    // cdtors
    VEC2() {}
    VEC2(f32 fx, f32 fy) {
        x = fx;
        y = fy;
    }

    // operators
    operator f32*() { return reinterpret_cast<f32*>(this); }
    operator const f32*() const { return reinterpret_cast<const f32*>(this); }

    // members
  public:
    /* base _VEC2 */ // size 0x08, offset 0x00
}; // size 0x08

struct VEC3 : public _VEC3 {
    // methods
  public:
    // cdtors
    VEC3() {}
    VEC3(f32 fx, f32 fy, f32 fz) {
        x = fx;
        y = fy;
        z = fz;
    }

    // operators
    operator VecPtr() { return reinterpret_cast<VecPtr>(this); }

    // members
  public:
    /* base _VEC3 */ // size 0x0c, offset 0x00
}; // size 0x0c

struct MTX34 : public _MTX34 {
    // methods
  public:
    // cdtors
    MTX34() {} // NOTE: must be non-defaulted

    // operators
    operator MtxPtr() { return mtx; }
    operator CMtxPtr() const { return mtx; }

    // members
  public:
    /* base _MTX34 */ // size 0x30, offset 0x00
}; // size 0x30

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

#endif // RVL_SDK_HBM_NW4HBM_MATH_TYPES_HPP
