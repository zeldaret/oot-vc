#include "math.h"
#include "revolution/mtx.h"

void C_MTXOrtho(Mtx44 mtx, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f) {
    f32 invrange;

    invrange = 1.0f / (r - l);
    mtx[0][0] = 2.0f * invrange;
    mtx[0][1] = 0.0f;
    mtx[0][2] = 0.0f;
    mtx[0][3] = invrange * -(r + l);

    invrange = 1.0f / (t - b);
    mtx[1][0] = 0.0f;
    mtx[1][1] = 2.0f * invrange;
    mtx[1][2] = 0.0f;
    mtx[1][3] = invrange * -(t + b);

    invrange = 1.0f / (f - n);
    mtx[2][0] = 0.0f;
    mtx[2][1] = 0.0f;
    mtx[2][2] = -1.0f * invrange;
    mtx[2][3] = -f * invrange;

    mtx[3][0] = 0.0f;
    mtx[3][1] = 0.0f;
    mtx[3][2] = 0.0f;
    mtx[3][3] = 1.0f;
}

//! TODO: name this
void fn_8009D008(Mtx44 m, Mtx44* src, Mtx44* dst) {
    f32 one = 1.0f;
    f32 zero = 0.0f;

#ifdef __MWERKS__ // clang-format off
    asm {
        lfs f1, zero
        lfs f0, one
        psq_st f1, 0x4(r3), 0, 0
        stfs f0, 0x0(r3)
        psq_st f1, 0xc(r3), 0, 0
        stfs f0, 0x14(r3)
        psq_st f1, 0x18(r3), 0, 0
        psq_st f1, 0x20(r3), 0, 0
        stfs f0, 0x28(r3)
        psq_st f1, 0x2c(r3), 0, 0
        psq_st f1, 0x34(r3), 0, 0
        stfs f0, 0x3c(r3)
    }
#endif
}

ASM void PSMTX44Concat(const register Mtx44 mA, const register Mtx44 mB, register Mtx44 mAB) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    psq_l   FP0, 0(mA), 0, 0
    psq_l   FP2, 0(mB), 0, 0
    ps_muls0    FP6, FP2, FP0
    psq_l   FP3, 16(mB), 0, 0
    psq_l   FP4, 32(mB), 0, 0
    ps_madds1   FP6, FP3, FP0, FP6
    psq_l   FP1, 8(mA), 0, 0
    psq_l   FP5, 48(mB), 0, 0
    ps_madds0   FP6, FP4, FP1, FP6
    psq_l   FP0, 16(mA), 0, 0
    ps_madds1   FP6, FP5, FP1, FP6
    psq_l   FP1, 24(mA), 0, 0
    ps_muls0    FP8, FP2, FP0
    ps_madds1   FP8, FP3, FP0, FP8
    psq_l   FP0, 32(mA), 0, 0
    ps_madds0   FP8, FP4, FP1, FP8
    ps_madds1   FP8, FP5, FP1, FP8
    psq_l   FP1, 40(mA), 0, 0
    ps_muls0    FP10, FP2, FP0
    ps_madds1   FP10, FP3, FP0, FP10
    psq_l   FP0, 48(mA), 0, 0
    ps_madds0   FP10, FP4, FP1, FP10
    ps_madds1   FP10, FP5, FP1, FP10
    psq_l   FP1, 56(mA), 0, 0
    ps_muls0    FP12, FP2, FP0
    psq_l   FP2, 8(mB), 0, 0
    ps_madds1   FP12, FP3, FP0, FP12
    psq_l   FP0, 0(mA), 0, 0
    ps_madds0   FP12, FP4, FP1, FP12
    psq_l   FP3, 24(mB), 0, 0
    ps_madds1   FP12, FP5, FP1, FP12
    psq_l   FP1, 8(mA), 0, 0
    ps_muls0    FP7, FP2, FP0
    psq_l   FP4, 40(mB), 0, 0
    ps_madds1   FP7, FP3, FP0, FP7
    psq_l   FP5, 56(mB), 0, 0
    ps_madds0   FP7, FP4, FP1, FP7
    psq_l   FP0, 16(mA), 0, 0
    ps_madds1   FP7, FP5, FP1, FP7
    psq_l   FP1, 24(mA), 0, 0
    ps_muls0    FP9, FP2, FP0
    psq_st  FP6, 0(mAB), 0, 0
    ps_madds1   FP9, FP3, FP0, FP9
    psq_l   FP0, 32(mA), 0, 0
    ps_madds0   FP9, FP4, FP1, FP9
    psq_st  FP8, 16(mAB), 0, 0
    ps_madds1   FP9, FP5, FP1, FP9
    psq_l   FP1, 40(mA), 0, 0
    ps_muls0    FP11, FP2, FP0
    psq_st  FP10, 32(mAB), 0, 0
    ps_madds1   FP11, FP3, FP0, FP11
    psq_l   FP0, 48(mA), 0, 0
    ps_madds0   FP11, FP4, FP1, FP11
    psq_st  FP12, 48(mAB), 0, 0
    ps_madds1   FP11, FP5, FP1, FP11
    psq_l   FP1, 56(mA), 0, 0
    ps_muls0    FP13, FP2, FP0
    psq_st  FP7, 8(mAB), 0, 0
    ps_madds1   FP13, FP3, FP0, FP13
    psq_st  FP9, 24(mAB), 0, 0
    ps_madds0   FP13, FP4, FP1, FP13
    psq_st  FP11, 40(mAB), 0, 0
    ps_madds1   FP13, FP5, FP1, FP13
    psq_st  FP13, 56(mAB), 0, 0
    blr
#endif // clang-format on
}

//! TODO: name this
ASM void fn_8009D140(Mtx44 m, Mtx44* src, Mtx44* dst){
#ifdef __MWERKS__ // clang-format off
    nofralloc

    psq_l f0, 0x0(r3), 0, 0
    psq_l f1, 0x10(r3), 0, 0
    ps_merge00 f4, f0, f1
    psq_l f2, 0x8(r3), 0, 0
    psq_st f4, 0x0(r4), 0, 0
    ps_merge11 f5, f0, f1
    psq_l f3, 0x18(r3), 0, 0
    psq_st f5, 0x10(r4), 0, 0
    ps_merge00 f4, f2, f3
    psq_l f0, 0x20(r3), 0, 0
    psq_st f4, 0x20(r4), 0, 0
    ps_merge11 f5, f2, f3
    psq_l f1, 0x30(r3), 0, 0
    psq_st f5, 0x30(r4), 0, 0
    ps_merge00 f4, f0, f1
    psq_l f2, 0x28(r3), 0, 0
    psq_st f4, 0x8(r4), 0, 0
    ps_merge11 f5, f0, f1
    psq_l f3, 0x38(r3), 0, 0
    psq_st f5, 0x18(r4), 0, 0
    ps_merge00 f4, f2, f3
    psq_st f4, 0x28(r4), 0, 0
    ps_merge11 f5, f2, f3
    psq_st f5, 0x38(r4), 0, 0
    blr
#endif
}

ASM void PSMTX44MultVecNoW(Mtx44 m, Mtx44* src, Mtx44* dst) {
#ifdef __MWERKS__ // clang-format off
    nofralloc

    psq_l f0, 0x0(r3), 0, 0
    psq_l f6, 0x0(r4), 0, 0
    psq_l f2, 0x10(r3), 0, 0
    ps_mul f8, f0, f6
    psq_l f4, 0x20(r3), 0, 0
    ps_mul f10, f2, f6
    psq_l f7, 0x8(r4), 1, 0
    ps_mul f12, f4, f6
    psq_l f3, 0x18(r3), 0, 0
    ps_sum0 f8, f8, f8, f8
    psq_l f5, 0x28(r3), 0, 0
    ps_sum0 f10, f10, f10, f10
    psq_l f1, 0x8(r3), 0, 0
    ps_sum0 f12, f12, f12, f12
    ps_madd f9, f1, f7, f8
    psq_st f9, 0x0(r5), 1, 0
    ps_madd f11, f3, f7, f10
    psq_st f11, 0x4(r5), 1, 0
    ps_madd f13, f5, f7, f12
    psq_st f13, 0x8(r5), 1, 0
    blr
#endif // clang-format on
}
