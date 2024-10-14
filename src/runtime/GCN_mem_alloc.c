#include "emulator/errordisplay.h"
#include "macros.h"
#include "revolution/os.h"

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

void Init(s32 size) {
    InitInline();
    OSAllocFromHeap(__OSCurrHeap, size);
}

WEAK void __sys_free(void* ptr) {
    InitInline();
    OSFreeToHeap(__OSCurrHeap, ptr);
}
