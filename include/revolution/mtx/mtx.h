#ifndef _RVL_SDK_MTX_MTX_H
#define _RVL_SDK_MTX_MTX_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef f32 Mtx23[2][3];
typedef f32 Mtx[3][4];
typedef f32 Mtx44[4][4];
typedef f32 (*Mtx44Ptr)[4];

typedef struct Vec {
    f32 x, y, z;
} Vec;

typedef struct Vec2 {
    f32 x, y;
} Vec2;

typedef struct Quaternion {
    f32 x, y, z, w;
} Quaternion;

typedef Vec2* Vec2Ptr;
typedef Vec* VecPtr;
typedef f32 (*Mtx23Ptr)[3];
typedef f32 (*MtxPtr)[4];
typedef f32 (*Mtx43Ptr)[3];
typedef f32 (*Mtx44Ptr)[4];

// not official names, just what i thought would make sense
typedef const f32 (*CMtx23Ptr)[3];
typedef const f32 (*CMtxPtr)[4];
typedef const f32 (*CMtx43Ptr)[3];
typedef const f32 (*CMtx44Ptr)[4];

void PSMTXIdentity(Mtx);
void PSMTXCopy(const Mtx, Mtx);
void PSMTXConcat(const Mtx, const Mtx, Mtx);
void PSMTXConcatArray(const Mtx, const Mtx, Mtx, u32);
void PSMTXTranspose(const Mtx, Mtx);
void PSMTXInverse(const Mtx mA, Mtx mB);
void PSMTXInvXpose(const Mtx, Mtx);
void PSMTXRotRad(Mtx m, char axis, f32 rad);
void PSMTXRotTrig(register Mtx m, register char axis, register f32 sinA, register f32 cosA);
void PSMTXRotAxisRad(Mtx, const Vec*, f32);
void PSMTXTrans(Mtx, f32, f32, f32);
void PSMTXTransApply(const Mtx, Mtx, f32, f32, f32);
void PSMTXScale(Mtx, f32, f32, f32);
void PSMTXScaleApply(const register Mtx src, register Mtx dst, register f32 xS, register f32 yS, register f32 zS);
void PSMTXQuat(Mtx, const Quaternion*);

void C_MTXLookAt(Mtx, const Vec*, const Vec*, const Vec*);
void C_MTXLightFrustum(Mtx, f32, f32, f32, f32, f32, f32, f32, f32, f32);
void C_MTXLightPerspective(Mtx, f32, f32, f32, f32, f32, f32);
void C_MTXLightOrtho(Mtx, f32, f32, f32, f32, f32, f32, f32, f32);

#ifdef __cplusplus
}
#endif

#endif
