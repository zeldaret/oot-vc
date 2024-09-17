#include "macros.h"
#include "revolution/base.h"
#include "revolution/os.h"

ASM u32 PPCMfmsr(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mfmsr r3
    blr
#endif // clang-format on
}

ASM void PPCMtmsr(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtmsr val
    blr
#endif // clang-format on
}

ASM u32 PPCMfhid0(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mfhid0 r3
    blr
#endif // clang-format on
}

ASM void PPCMthid0(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mthid0 val
    blr
#endif // clang-format on
}

ASM u32 PPCMfl2cr(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mfl2cr r3
    blr
#endif // clang-format on
}

ASM void PPCMtl2cr(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtl2cr val
    blr
#endif // clang-format on
}

ASM void PPCMtdec(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtdec val
    blr
#endif // clang-format on
}

ASM void PPCSync(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    sc
    blr
#endif // clang-format on
}

ASM void PPCHalt(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    sync

loop:
    nop
    li r3, 0
    nop
    b loop
#endif // clang-format on
}

ASM void PPCMtmmcr0(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtmmcr0 val
    blr
#endif // clang-format on
}

ASM void PPCMtmmcr1(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtmmcr1 val
    blr
#endif // clang-format on
}

ASM void PPCMtpmc1(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtpmc1 val
    blr
#endif // clang-format on
}

ASM void PPCMtpmc2(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtpmc2 val
    blr
#endif // clang-format on
}

ASM void PPCMtpmc3(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtpmc3 val
    blr
#endif // clang-format on
}

ASM void PPCMtpmc4(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtpmc4 val
    blr
#endif // clang-format on
}

u32 PPCMffpscr(void) {
#ifdef __MWERKS__ // clang-format off
    register u64 fpscr;
    asm {
        mffs f31
        stfd f31, fpscr
    }

    return fpscr;
#endif // clang-format on
}

void PPCMtfpscr(register u32 val) {
#ifdef __MWERKS__ // clang-format off
    register struct {
        f32 tmp;
        f32 data;
    } fpscr;

    asm {
        li r4, 0
        stw val, fpscr.data
        stw r4, fpscr.tmp
        lfd f31, fpscr.tmp
        mtfs f31
    }
#endif // clang-format on
}

ASM u32 PPCMfhid2(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mfspr r3, 0x398
    blr
#endif // clang-format on
}

ASM void PPCMthid2(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtspr 0x398, val
    blr
#endif // clang-format on
}

ASM void PPCMtwpar(register u32 val) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtwpar r3
    blr
#endif // clang-format on
}

void PPCDisableSpeculation(void) { PPCMthid0(PPCMfhid0() | HID0_SPD); }

ASM void PPCSetFpNonIEEEMode(void) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtfsb1 29
    blr
#endif // clang-format on
}

void PPCMthid4(register u32 val) {
    if (val & HID4_H4A) {
#ifdef __MWERKS__ // clang-format off
        asm {
            mtspr 0x3F3, val
        }
#endif // clang-format on
    } else {
        OSReport("H4A should not be cleared because of Broadway errata.\n");
        val |= HID4_H4A;
#ifdef __MWERKS__ // clang-format off
        asm {
            mtspr 0x3F3, val
        }
#endif // clang-format on
    }
}
