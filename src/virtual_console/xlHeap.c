#include "xlHeap.h"

GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/func_800809B0.s")

GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapBlockCacheAdd.s")

GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapBlockCacheClear.s")

GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapBlockCacheReset.s")

GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapCompact.s")

GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapTake.s")

GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapFree.s")

s32 xlHeapCopy(void *dst, void *src, int len) {
    if(((u32)src % 4) == 0 && ((u32)dst % 4) == 0){
        for(; len >= 0x40; len -= 0x40) {
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
        }

        for(; len >= 0x20; len -= 0x20) {
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
        }

        for(; len >= 4; len -= 0x4) {
            *((u32*)dst)++ = *((u32*)src)++;
        }
    }

    for(; len >= 0x10; len -= 0x10) {
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
    }

    for(; len >= 8; len -= 0x8) {
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
    }

    for(; len > 0; len--) {
        *((u8*)dst)++ = *((u8*)src)++;
    }

    return 1;

}

s32 xlHeapFill(u8 *src, s32 len, u8 fill) {
    s32 cnt;
    for(cnt = len; cnt > 0x10; cnt -= 0x10) {
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
    }

    for(; cnt > 0; cnt--) {
        *src++ = fill;
    }

    return 1;
}

s32 xlHeapFill32(u32 *src, s32 len, u32 fill) {
    s32 cnt;

    for(cnt = len >> 2; cnt > 0x10; cnt -= 0x10){
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
        *src++ = fill;
    }

    for(; cnt > 0; cnt--) {
        *src++ = fill;
    }

    return 1;
}

extern void *gpHeapBlockFirst[2];
extern void *gpHeapBlockLast[2];

s32 xlHeapCompact(s32);

s32 __xlHeapGetFree(s32 heap, u32 *free) {
    u32 total_free;
    u32 *hdr_p;
    u32 hdr;
    u32 blk_size;

    if(!xlHeapCompact(heap)) {
        return 0;
    }

    for(hdr_p = gpHeapBlockFirst[heap], total_free = 0; hdr_p < gpHeapBlockLast[heap];){
        hdr = *hdr_p;
        blk_size = hdr & 0xFFFFFF;

        if((hdr & 0x3F) != (hdr >> 0x1A)) {
            return 0;
        }

        if(hdr & 0x1000000) {
            total_free += blk_size * 4;
        }

        hdr_p += blk_size + 1;
    }
    *free = total_free;
    return 1;


}

s32 xlHeapGetFree(u32 *free) {
    return __xlHeapGetFree(0, free);
}

GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapSetup.s")

void func_8008B114(void *ptr);
void func_8008B11C(void *ptr);
extern void *lbl_8025D1B8[2];
extern void *lbl_8025D1BC;

s32 xlHeapReset(void) {
    func_8008B114(lbl_8025D1B8[0]);
    func_8008B11C(lbl_8025D1B8[1]);
    return 1;
}