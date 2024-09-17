#include "macros.h"
#include "revolution/base.h"
#include "revolution/db.h"
#include "revolution/os.h"

ASM void DCEnable(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    sync
    mfhid0 r3
    ori r3, r3, 0x4000
    mthid0 r3

    blr
#endif // clang-format on
}

ASM void DCInvalidateRange(register const void* buf, register u32 len){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    cmplwi len, 0
    blelr 

    clrlwi r5, buf, 27
    add len, len, r5
    addi len, len, 31
    srwi len, len, 5
    mtctr len

do_invalidate:
    dcbi 0, buf
    addi buf, buf, 32
    bdnz do_invalidate

    blr
#endif // clang-format on
}

ASM void DCFlushRange(register const void* buf, register u32 len){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    cmplwi len, 0
    blelr 

    clrlwi r5, buf, 27
    add len, len, r5
    addi len, len, 31
    srwi len, len, 5
    mtctr len

do_flush:
    dcbf 0, buf
    addi buf, buf, 32
    bdnz do_flush
    sc

    blr
#endif // clang-format on
}

ASM void DCStoreRange(register const void* buf, register u32 len){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    cmplwi len, 0
    blelr 

    clrlwi r5, buf, 27
    add len, len, r5
    addi len, len, 31
    srwi len, len, 5
    mtctr len

do_store:
    dcbst 0, buf
    addi buf, buf, 32
    bdnz do_store
    sc

    blr
#endif // clang-format on
}

ASM void DCFlushRangeNoSync(register const void* buf, register u32 len){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    cmplwi len, 0
    blelr 

    clrlwi r5, buf, 27
    add len, len, r5
    addi len, len, 31
    srwi len, len, 5
    mtctr len

do_flush:
    dcbf 0, buf
    addi buf, buf, 32
    bdnz do_flush

    blr
#endif // clang-format on
}

ASM void DCZeroRange(register const void* buf, register u32 len){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    cmplwi len, 0
    blelr 

    clrlwi r5, buf, 27
    add len, len, r5
    addi len, len, 31
    srwi len, len, 5
    mtctr len

do_zero:
    dcbz 0, buf
    addi buf, buf, 32
    bdnz do_zero

    blr
#endif // clang-format on
}

ASM void ICInvalidateRange(register const void* buf, register u32 len){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    cmplwi len, 0
    blelr 

    clrlwi r5, buf, 27
    add len, len, r5
    addi len, len, 31
    srwi len, len, 5
    mtctr len

do_invalidate:
    icbi 0, buf
    addi buf, buf, 32
    bdnz do_invalidate
    
    sync
    isync

    blr
#endif // clang-format on
}

ASM void ICFlashInvalidate(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    mfhid0 r3
    ori r3, r3, 0x800
    mthid0 r3

    blr
#endif // clang-format on
}

ASM void ICEnable(void) {
#ifdef __MWERKS__ // clang-format off
    nofralloc

    isync
    mfhid0 r3
    ori r3, r3, 0x8000
    mthid0 r3

    blr
#endif // clang-format on
}

static inline void L2Init(void) {
    u32 msr;

    msr = PPCMfmsr();
    __sync();
    PPCMtmsr(MSR_IR | MSR_DR);
    __sync();

    L2Disable();
    L2GlobalInvalidate();
    PPCMtmsr(msr);
}

static inline void L2Enable(void) {
    u32 l2cr = PPCMfl2cr();
    PPCMtl2cr((l2cr | L2CR_L2E) & ~L2CR_L2I);
}

static inline void L2Disable(void) {
    u32 l2cr;

    __sync();
    l2cr = PPCMfl2cr();
    PPCMtl2cr(l2cr & ~L2CR_L2E);
    __sync();
}

ASM void LCDisable(void) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    lis r3, 0xE0000000@ha
    li r4, 512
    mtctr r4
do_invalidate:
    dcbi 0, r3
    addi r3, r3, 32
    bdnz do_invalidate
    
    mfspr r4, 0x398
    rlwinm r4, r4, 0, 4, 2
    mtspr 0x398, r4

    blr
#endif // clang-format on
}

void L2GlobalInvalidate(void) {
    u32 l2cr;

    L2Disable();

    l2cr = PPCMfl2cr();
    PPCMtl2cr(l2cr | L2CR_L2I);

    while (PPCMfl2cr() & L2CR_L2IP) {}

    l2cr = PPCMfl2cr();
    PPCMtl2cr(l2cr & ~L2CR_L2I);

    while (PPCMfl2cr() & L2CR_L2IP) {
        DBPrintf(">>> L2 INVALIDATE : SHOULD NEVER HAPPEN\n");
    }
}

void DMAErrorHandler(u8 error, OSContext* ctx, u32 dsisr, u32 dar, ...) {
    u32 hid2 = PPCMfhid2();

    OSReport("Machine check received\n");
    OSReport("HID2 = 0x%x   SRR1 = 0x%x\n", hid2, ctx->srr1);

    if (!(hid2 & (HID2_DCHERR | HID2_DNCERR | HID2_DCMERR | HID2_DQOERR)) || !(ctx->srr1 & 0x200000)) {
        OSReport("Machine check was not DMA/locked cache related\n");
        OSDumpContext(ctx);
        PPCHalt();
    }

    OSReport("DMAErrorHandler(): An error occurred while processing DMA.\n");
    OSReport("The following errors have been detected and cleared :\n");

    if (hid2 & HID2_DCHERR) {
        OSReport("\t- Requested a locked cache tag that was already in the cache\n");
    }
    if (hid2 & HID2_DNCERR) {
        OSReport("\t- DMA attempted to access normal cache\n");
    }
    if (hid2 & HID2_DCMERR) {
        OSReport("\t- DMA missed in data cache\n");
    }
    if (hid2 & HID2_DQOERR) {
        OSReport("\t- DMA queue overflowed\n");
    }

    PPCMthid2(hid2);
}

void __OSCacheInit(void) {
    if (!(PPCMfhid0() & HID0_ICE)) {
        ICEnable();
        DBPrintf("L1 i-caches initialized\n");
    }

    if (!(PPCMfhid0() & HID0_DCE)) {
        DCEnable();
        DBPrintf("L1 d-caches initialized\n");
    }

    if (!(PPCMfl2cr() & L2CR_L2E)) {
        L2Init();
        L2Enable();
        DBPrintf("L2 cache initialized\n");
    }

    OSSetErrorHandler(OS_ERR_MACHINE_CHECK, DMAErrorHandler);
    DBPrintf("Locked cache machine check handler installed\n");
}
