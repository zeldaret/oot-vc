#include "xlHeap.h"

s32 xlHeapCompact(s32);
void xlHeapBlockCacheClear(s32 heap);
s32 xlHeapBlockCacheAdd(s32 heap, void *blk);
s32 func_800809B0(s32, s32, void **, s32*);
void OSSetArena1Lo(void *ptr);
void OSSetArena1Hi(void *ptr);
void OSSetArena2Lo(void *ptr);
void OSSetArena2Hi(void *ptr);
void *OSGetArena1Lo(void);
void *OSGetArena1Hi(void);
void *OSGetArena2Lo(void);
void *OSGetArena2Hi(void);

extern void *lbl_8025D1B8[2]; 
extern void *gpHeapBlockFirst[2];
extern void *gpHeapBlockLast[2];
extern s32 lbl_8025D1C8[2];
extern s32 gnHeapTakeCacheCount[2];

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/func_800809B0.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapBlockCacheAdd.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapBlockCacheClear.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapBlockCacheReset.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapCompact.s")

#ifdef NON_EQUIVALENT
s32 xlHeapTake(void **ptr, size_t size) {
    s32 align;
    s32 heap = (size >> 30) & 1;
    u32 blk_cnt;
    s32 blk_found;
    s32 cont;
    s32 local_34;
    u32 var;
    u32 *blk;

    switch(size & 0x30000000) {
        case 0:
            align = 0x3;
            break;
        case 0x10000000:
            align = 0x7;
            break;
        case 0x20000000:
            align = 0xF;
            break;
        case 0x30000000:
            align = 0x1F;
            break;
        default:
            align = 0;
            break;
    }


    blk_cnt = ((size & ~0x70000000) + align) >> 2;

    if(blk_cnt >= 1 || blk_cnt > 0x1000000){
        return 0;
    }

    for(var = 0; var < 8; var++){
        if(!func_800809B0(heap, blk_cnt, (void**)&blk, &local_34)) {
            blk = gpHeapBlockFirst[heap];
            while(blk < gpHeapBlockLast[heap]) {
                local_34 = *blk & 0xFFFFFF;
                if((*blk >> 0x1A) != (*blk & 0x3F)) {
                    return 0;
                }

                if((*blk & 0x1000000) && blk_cnt <= local_34) {
                    blk_found = 1;
                    break;
                }
                blk += local_34;
            }
        } else {
            blk_found = 1;
        }

        if(blk_found){
            if(blk_cnt == local_34 - 1){
                blk_cnt++;
            }

            if(blk_cnt < local_34) {
                u32 uVar6 = (local_34 - blk_cnt) - 1;
                u32 uVar5 = blk[local_34 + 1] & 0xFFFFFF;
                u32 *puVar7 = blk + blk_cnt + 1;
                if(uVar5 != 0 && (blk[local_34 + 1] & 0x1000000)) {
                    xlHeapBlockCacheClear(heap);
                    uVar6 = uVar5 + 1;
                }
                *puVar7 = (uVar6 << 0x1A) | uVar6 | 0x1000000;
                xlHeapBlockCacheAdd(heap, puVar7);
            }
            *blk = (blk_cnt << 0x1A) | blk_cnt | 0x2000000;
            gnHeapTakeCacheCount[heap]++;
            while((u32)++blk & align) {
                *blk = 0x1234ABCD;
            }
            *ptr = blk;
            return 1;
        }

        if(!xlHeapCompact(heap)) {
            return 0;
        }
    }

    return 0;

}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapTake.s")
#endif

#ifdef NON_EQUIVALENT
s32 xlHeapFree(void **ptr) {
    s32 heap = 0;
    s32 *hdr_p;
    u32 hdr;
    s32 size;
    s32 size2;
    s32 i;

    if(gpHeapBlockFirst[0] > *ptr || gpHeapBlockLast[0] < *ptr) {
        heap++;
        if(gpHeapBlockFirst[1] > *ptr || gpHeapBlockLast[1] < *ptr) {
            heap++;
        }
    }

    if(heap == 2){
        return 0;
    }

    for(hdr_p = (s32*)*ptr - 1; *hdr_p == 0x1234ABCD; hdr_p--){

    }

    size = *hdr_p & 0xFFFFFF; // r28

    if(*hdr_p & 0x1000000 || *hdr_p & 0x2000000) {
        return 0;
    }

    if((*hdr_p >> 0x1A) != (*hdr_p & 0x3F)) {
        return 0;
    }

    hdr_p += size;
    size2 = *++hdr_p & 0xFFFFFF; // r27
    if(size != 0 && (*hdr_p & 0x1000000)) {
        xlHeapBlockCacheClear(heap);
        size = size + size2 + 1;
    }

    *hdr_p = (size << 0x1A) | size | 0x1000000;
    xlHeapBlockCacheAdd(heap, hdr_p);

    lbl_8025D1C8[heap]++;
    *ptr = NULL;
    
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapFree.s")
#endif

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

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapSetup.s")

s32 xlHeapReset(void) {
    OSSetArena1Lo(lbl_8025D1B8[0]);
    OSSetArena2Lo(lbl_8025D1B8[1]);
    return 1;
}