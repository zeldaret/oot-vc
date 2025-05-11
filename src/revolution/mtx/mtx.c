#include "revolution/mtx.h"
#include "macros.h"
#include "math.h"

static f32 Unit01[] = {0.0f, 1.0f};

void PSMTXIdentity(register Mtx m) {
    register f32 zero_c;
    register f32 one_c;
    register f32 c_01;
    register f32 c_10;

    // fixes float ordering issue
    (void)1.0f;

    zero_c = 0.0f;
    one_c = 1.0f;

#ifdef __MWERKS__ // clang-format off
    asm {
        psq_st zero_c, 8(m), 0, 0
        ps_merge01 c_01, zero_c, one_c
        psq_st zero_c, 24(m), 0, 0
        ps_merge10 c_10, one_c, zero_c
        psq_st zero_c, 32(m), 0, 0
        psq_st c_01, 16(m), 0, 0
        psq_st c_10, 0(m), 0, 0
        psq_st c_10, 40(m), 0, 0
    }
#endif // clang-format on
}

ASM void PSMTXCopy(const Mtx, Mtx){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    psq_l f0, 0x0(r3), 0, 0
    psq_st f0, 0x0(r4), 0, 0
    psq_l f1, 0x8(r3), 0, 0
    psq_st f1, 0x8(r4), 0, 0
    psq_l f2, 0x10(r3), 0, 0
    psq_st f2, 0x10(r4), 0, 0
    psq_l f3, 0x18(r3), 0, 0
    psq_st f3, 0x18(r4), 0, 0
    psq_l f4, 0x20(r3), 0, 0
    psq_st f4, 0x20(r4), 0, 0
    psq_l f5, 0x28(r3), 0, 0
    psq_st f5, 0x28(r4), 0, 0
    blr
#endif // clang-format on
}

ASM void PSMTXConcat(const register Mtx mA, const register Mtx mB, register Mtx mAB) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    stwu    r1, -64(r1);
    psq_l   FP0, 0(mA), 0, 0;
    stfd    fp14, 8(r1);
    psq_l   FP6, 0(mB), 0, 0;
    addis   r6, 0, Unit01@ha;
    psq_l   FP7, 8(mB), 0, 0;
    stfd    fp15, 16(r1)
    addi    r6, r6, Unit01@l;
    stfd    fp31, 40(r1)
    psq_l   FP8, 16(mB), 0, 0
    ps_muls0 FP12, FP6, FP0
    psq_l   FP2, 16(mA), 0, 0
    ps_muls0 FP13, FP7, FP0
    psq_l   FP31, 0(r6), 0, 0
    ps_muls0 FP14, FP6, FP2
    psq_l   FP9, 24(mB), 0, 0
    ps_muls0 FP15, FP7, FP2
    psq_l   FP1, 8(mA), 0, 0
    ps_madds1 FP12, FP8, FP0, FP12
    psq_l   FP3, 24(mA), 0, 0
    ps_madds1 FP14, FP8, FP2, FP14
    psq_l   FP10, 32(mB), 0, 0
    ps_madds1 FP13, FP9, FP0, FP13
    psq_l   FP11, 40(mB), 0, 0
    ps_madds1 FP15, FP9, FP2, FP15
    psq_l   FP4, 32(mA), 0, 0
    psq_l   FP5, 40(mA), 0, 0
    ps_madds0 FP12, FP10, FP1, FP12
    ps_madds0 FP13, FP11, FP1, FP13
    ps_madds0 FP14, FP10, FP3, FP14
    ps_madds0 FP15, FP11, FP3, FP15
    psq_st  FP12, 0(mAB), 0, 0

    ps_muls0 FP2, FP6, FP4
    ps_madds1 FP13, FP31, FP1, FP13
    ps_muls0 FP0, FP7, FP4
    psq_st  FP14, 16(mAB), 0, 0
    ps_madds1 FP15, FP31, FP3, FP15

    psq_st  FP13, 8(mAB), 0, 0

    ps_madds1 FP2, FP8, FP4, FP2
    ps_madds1 FP0, FP9, FP4, FP0
    ps_madds0 FP2, FP10, FP5, FP2
    lfd    fp14, 8(r1)
    psq_st  FP15, 24(mAB), 0, 0
    ps_madds0 FP0, FP11, FP5, FP0
    psq_st  FP2, 32(mAB), 0, 0
    ps_madds1 FP0, FP31, FP5, FP0
    lfd    fp15, 16(r1)
    psq_st  FP0, 40(mAB), 0, 0

    lfd    fp31, 40(r1)
    addi   r1, r1, 64

    blr
#endif // clang-format on
}

ASM void PSMTXInverse(const register Mtx mA, const register Mtx mB, register Mtx mAB){
#ifdef __MWERKS__ // clang-format off
    nofralloc
	psq_l f0, 0x0(r3), 1, 0
	psq_l f1, 0x4(r3), 0, 0
	psq_l f2, 0x10(r3), 1, 0
	ps_merge10 f6, f1, f0
	psq_l f3, 0x14(r3), 0, 0
	psq_l f4, 0x20(r3), 1, 0
	ps_merge10 f7, f3, f2
	psq_l f5, 0x24(r3), 0, 0
	ps_mul f11, f3, f6
	ps_mul f13, f5, f7
	ps_merge10 f8, f5, f4
	ps_msub f11, f1, f7, f11
	ps_mul f12, f1, f8
	ps_msub f13, f3, f8, f13
	ps_mul f10, f3, f4
	ps_msub f12, f5, f6, f12
	ps_mul f9, f0, f5
	ps_mul f8, f1, f2
	ps_sub f6, f6, f6
	ps_msub f10, f2, f5, f10
	ps_mul f7, f0, f13
	ps_msub f9, f1, f4, f9
	ps_madd f7, f2, f12, f7
	ps_msub f8, f0, f3, f8
	ps_madd f7, f4, f11, f7
	ps_cmpo0 cr0, f7, f6
	bne L_8009CCC4
	li r3, 0x0
	blr
L_8009CCC4:
	fres f0, f7
	ps_add f6, f0, f0
	ps_mul f5, f0, f0
	ps_nmsub f0, f7, f5, f6
	lfs f1, 0xc(r3)
	ps_muls0 f13, f13, f0
	lfs f2, 0x1c(r3)
	ps_muls0 f12, f12, f0
	lfs f3, 0x2c(r3)
	ps_muls0 f11, f11, f0
	ps_merge00 f5, f13, f12
	ps_muls0 f10, f10, f0
	ps_merge11 f4, f13, f12
	ps_muls0 f9, f9, f0
	psq_st f5, 0x0(r4), 0, 0
	ps_mul f6, f13, f1
	psq_st f4, 0x10(r4), 0, 0
	ps_muls0 f8, f8, f0
	ps_madd f6, f12, f2, f6
	psq_st f10, 0x20(r4), 1, 0
	ps_nmadd f6, f11, f3, f6
	psq_st f9, 0x24(r4), 1, 0
	ps_mul f7, f10, f1
	ps_merge00 f5, f11, f6
	psq_st f8, 0x28(r4), 1, 0
	ps_merge11 f4, f11, f6
	psq_st f5, 0x8(r4), 0, 0
	ps_madd f7, f9, f2, f7
	psq_st f4, 0x18(r4), 0, 0
	ps_nmadd f7, f8, f3, f7
	li r3, 0x1
	psq_st f7, 0x2c(r4), 1, 0
	blr
#endif // clang-format on
} f32 fn_8009CDC4(s32 arg1, f64 arg8, f64 arg9);

//! TODO: actually C?
ASM void PSMTXRotRad(Mtx m, char axis, f32 rad) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
	stwu r1, -0x30(r1)
	mflr r0
	stw r0, 0x34(r1)
	stfd f31, 0x20(r1)
	psq_st f31, 0x28(r1), 0, 0
	stfd f30, 0x10(r1)
	psq_st f30, 0x18(r1), 0, 0
	fmr f30, f1
	stw r31, 0xc(r1)
	mr r31, r4
	stw r30, 0x8(r1)
	mr r30, r3
	bl sin
	frsp f31, f1
	fmr f1, f30
	bl cos
	frsp f2, f1
	mr r3, r30
	fmr f1, f31
	extsb r4, r31
	bl PSMTXRotTrig
	psq_l f31, 0x28(r1), 0, 0
	lfd f31, 0x20(r1)
	psq_l f30, 0x18(r1), 0, 0
	lfd f30, 0x10(r1)
	lwz r31, 0xc(r1)
	lwz r0, 0x34(r1)
	lwz r30, 0x8(r1)
	mtlr r0
	addi r1, r1, 0x30
	blr
#endif // clang-format on
}

void PSMTXRotTrig(register Mtx m, register char axis, register f32 sinA, register f32 cosA) {
    register f32 fc0, fc1, nsinA;
    register f32 fw0, fw1, fw2, fw3;

#ifdef __MWERKS__ // clang-format off
	asm {
		frsp        sinA, sinA
		frsp        cosA, cosA
	}
#endif // clang-format on

    fc0 = 0.0f;
    fc1 = 1.0f;

#ifdef __MWERKS__ // clang-format off
	asm {
		ori         axis, axis, 0x20
		ps_neg      nsinA, sinA
		cmplwi      axis, 'x'
		beq         _case_x
		cmplwi      axis, 'y'
		beq         _case_y
		cmplwi      axis, 'z'
		beq         _case_z
		b           _end

	_case_x:
		psq_st      fc1,  0(m), 1, 0
		psq_st      fc0,  4(m), 0, 0
		ps_merge00  fw0, sinA, cosA
		psq_st      fc0, 12(m), 0, 0
		ps_merge00  fw1, cosA, nsinA
		psq_st      fc0, 28(m), 0, 0
		psq_st      fc0, 44(m), 1, 0
		psq_st      fw0, 36(m), 0, 0
		psq_st      fw1, 20(m), 0, 0
		b           _end;

	_case_y:
		ps_merge00  fw0, cosA, fc0
		ps_merge00  fw1, fc0, fc1
		psq_st      fc0, 24(m), 0, 0
		psq_st      fw0,  0(m), 0, 0
		ps_merge00  fw2, nsinA, fc0
		ps_merge00  fw3, sinA, fc0
		psq_st      fw0, 40(m), 0, 0;
		psq_st      fw1, 16(m), 0, 0;
		psq_st      fw3,  8(m), 0, 0;
		psq_st      fw2, 32(m), 0, 0;
		b           _end;

	_case_z:
		psq_st      fc0,  8(m), 0, 0
		ps_merge00  fw0, sinA, cosA
		ps_merge00  fw2, cosA, nsinA
		psq_st      fc0, 24(m), 0, 0
		psq_st      fc0, 32(m), 0, 0
		ps_merge00  fw1, fc1, fc0
		psq_st      fw0, 16(m), 0, 0
		psq_st      fw2,  0(m), 0, 0
		psq_st      fw1, 40(m), 0, 0

	_end:

	}
#endif // clang-format on
}

void PSMTXTrans(register Mtx m, register f32 xT, register f32 yT, register f32 zT) {
    register f32 c0 = 0.0F;
    register f32 c1 = 1.0F;

#ifdef __MWERKS__ // clang-format off
    asm {
        stfs        xT,     12(m)
        stfs        yT,     28(m)
        psq_st      c0,      4(m), 0, 0
        psq_st      c0,     32(m), 0, 0
        stfs        c0,     16(m)
        stfs        c1,     20(m)
        stfs        c0,     24(m)
        stfs        c1,     40(m)
        stfs        zT,     44(m)
        stfs        c1,      0(m)
    }
#endif // clang-format on
}

ASM void PSMTXTransApply(const register Mtx src, register Mtx dst, register f32 xT, register f32 yT, register f32 zT) {
#ifdef __MWERKS__ // clang-format off
    nofralloc;
    psq_l       fp4, 0(src),        0, 0
    frsp        xT, xT
    psq_l       fp5, 8(src),        0, 0
    frsp        yT, yT
    psq_l       fp7, 24(src),       0, 0
    frsp        zT, zT
    psq_l       fp8, 40(src),       0, 0
    psq_st      fp4, 0(dst),        0, 0
    ps_sum1     fp5, xT, fp5, fp5
    psq_l       fp6, 16(src),       0, 0   
    psq_st      fp5, 8(dst),        0, 0
    ps_sum1     fp7, yT, fp7, fp7
    psq_l       fp9, 32(src),       0, 0
    psq_st      fp6, 16(dst),       0, 0
    ps_sum1     fp8, zT, fp8, fp8
    psq_st      fp7, 24(dst),       0, 0
    psq_st      fp9, 32(dst),       0, 0
    psq_st      fp8, 40(dst),       0, 0
    blr
#endif // clang-format on
}

void PSMTXScale(register Mtx m, register f32 xS, register f32 yS, register f32 zS) {
    register f32 c0 = 0.0F;

#ifdef __MWERKS__ // clang-format off
    asm {
        stfs        xS,      0(m)
        psq_st      c0,      4(m), 0, 0
        psq_st      c0,     12(m), 0, 0
        stfs        yS,     20(m)
        psq_st      c0,     24(m), 0, 0
        psq_st      c0,     32(m), 0, 0
        stfs        zS,     40(m)
        stfs        c0,     44(m)
    }
#endif // clang-format on
}
