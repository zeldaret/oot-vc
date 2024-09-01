#include "metrotrk/mem_TRK.h"
#include "macros.h"
#include "revolution/types.h"

static inline u8 ppc_readbyte1(const u8* ptr) {
    u32* alignedPtr = (u32*)((u32)ptr & ~3);
    return (u8)(*alignedPtr >> ((3 - ((u32)ptr - (u32)alignedPtr)) << 3));
}

static inline void ppc_writebyte1(u8* ptr, u8 val) {
    u32* alignedPtr = (u32*)((u32)ptr & ~3);
    u32 v = *alignedPtr;
    u32 uVar3 = 0xff << ((3 - ((u32)ptr - (u32)alignedPtr)) << 3);
    u32 iVar1 = (3 - ((u32)ptr - (u32)alignedPtr)) << 3;
    *alignedPtr = (v & ~uVar3) | (uVar3 & (val << iVar1));
}

void TRK_fill_mem(void* dest, int val, size_t count);

INIT void* TRK_memset(void* dest, int val, size_t count) {
    TRK_fill_mem(dest, val, count);
    return dest;
}

INIT void* TRK_memcpy(void* dest, const void* src, size_t count) {
    u8* s;
    u8* d;

    for (s = (u8*)src - 1, d = (u8*)dest - 1, count++; --count;) {
        *++d = *++s;
    }
}
