#include "revolution/os.h"
#include "macros.h"

//! TODO: document
fn_80063C28(s32 handle, void *p);

static inline void InitInline(void) {
    void* arenaLo;
    void* arenaHi;

    if (__OSCurrHeap == -1) {
        OSReport("GCN_Mem_Alloc.c : InitDefaultHeap. No Heap Available\n");
        OSReport("Metrowerks CW runtime library initializing default heap\n");

        arenaLo = OSGetArenaLo();
        arenaHi = OSGetArenaHi();
        arenaLo = OSInitAlloc(arenaLo, arenaHi, 1);
        OSSetArenaLo(arenaLo);

        arenaLo = (void*)OSRoundUp32B(arenaLo);
        arenaHi = (void*)OSRoundDown32B(arenaHi);
        OSSetCurrentHeap(OSCreateHeap(arenaLo, arenaHi));
        OSSetArenaLo(arenaLo = arenaHi);
    }
}

void Init(void *ptr) {
    InitInline();
    fn_80063C28(__OSCurrHeap, ptr);
}

WEAK void __sys_free(void *ptr) {
    InitInline();
    OSFreeToHeap(__OSCurrHeap, ptr);
}
