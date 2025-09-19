#include "emulator/errordisplay.h"
#include "macros.h"
#include "revolution/os.h"

static inline void InitDefaultHeap(void) {
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

void __sys_alloc(s32 size) {
    InitDefaultHeap();
    OSAllocFromHeap(__OSCurrHeap, size);
}

WEAK void __sys_free(void* ptr) {
    InitDefaultHeap();
    OSFreeToHeap(__OSCurrHeap, ptr);
}
