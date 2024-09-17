#include "revolution/db.h"
#include "macros.h"
#include "revolution/base.h"
#include "revolution/os.h"

bool DBVerbose;
OSDebugInterface* __DBInterface;

void DBInit(void) {
    __DBInterface = (OSDebugInterface*)OSPhysicalToCached(OS_PHYS_DEBUG_INTERFACE);
    __DBInterface->exceptionHook = OSCachedToPhysical(__DBExceptionDestination);
    DBVerbose = true;
}

void __DBExceptionDestinationAux(void) {
    const void* physCtx = (void*)OS_PHYS_CURRENT_CONTEXT_PHYS;
    OSContext* ctx = (OSContext*)OSPhysicalToCached(*(u32*)physCtx);
    OSReport("DBExceptionDestination\n");
    OSDumpContext(ctx);
    PPCHalt();
}

ASM void __DBExceptionDestination(void) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    
    // Set up MMU
    mfmsr r3
    ori r3, r3, (MSR_IR | MSR_DR)
    mtmsr r3

    b __DBExceptionDestinationAux
#endif // clang-format on
}

bool __DBIsExceptionMarked(u8 exc) { return __DBInterface->exceptionMask & (1 << exc); }

void DBPrintf(const char* msg, ...) {
#pragma unused(msg)
}
