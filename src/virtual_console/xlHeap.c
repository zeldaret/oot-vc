#include "os.h"
#include "xlHeap.h"

s32 gnSizeHeap[2];
s32 gpHeap[2];
void* gpHeapBlockFirst[2];
void* gpHeapBlockLast[2];
s32 gnHeapTakeCacheCount[2];
s32 lbl_8025D1C8[2];
u32 lbl_8025D1C0[2];
void* gpHeapOS[2];
extern u32 *gapHeapBlockCache[2][11][32];

#define ALIGN(v, a) (((u32)(v) + a - 1) & ~(a - 1))
#define IS_FREE(v) ((v) & 0x1000000)
#define IS_TAKEN(v) ((v) & 0x2000000)
#define NUM_WORDS(v) ((s32)((v) & 0xFFFFFF))
#define CHKSUM_HI(v) ((u32)(v) >> 0x1A)
#define CHKSUM_LO(v) ((u32)(v) & 0x3F)

s32 xlHeapBlockCacheGet(s32 heap, s32 arg1, u32** arg2, s32* arg3) {
    s32 i;
    s32 idx;
    s32 cache_idx;
    s32 uVar6;

    if(arg1 < 8) {
        idx = 0;
    } else if(arg1 < 16) {
        idx = 1;
    } else if(arg1 < 32) {
        idx = 2;
    } else if(arg1 < 64) {
        idx = 3;
    } else if(arg1 < 128) {
        idx = 4;
    } else if(arg1 < 256) {
        idx = 5;
    } else if(arg1 < 512) {
        idx = 6;
    } else if(arg1 < 1024) {
        idx = 7;
    } else if(arg1 < 4096) {
        idx = 8;
    } else if(arg1 < 8192) {
        idx = 9;
    } else {
        idx = 10;
    }

    while(idx < 11) {
        cache_idx = -1;
        uVar6 = 0x1000000;
        for(i = 0; i < 32; i++) {
            if(gapHeapBlockCache[heap][idx][i] != NULL && NUM_WORDS(*gapHeapBlockCache[heap][idx][i]) < uVar6 &&
                NUM_WORDS(*gapHeapBlockCache[heap][idx][i]) >= arg1)
            {
                cache_idx = i;
                uVar6 = NUM_WORDS(*gapHeapBlockCache[heap][idx][i]);
            }
        }

        if(cache_idx >= 0) {
            *arg3 = uVar6;
            *arg2 = gapHeapBlockCache[heap][idx][cache_idx];
            gapHeapBlockCache[heap][idx][cache_idx] = NULL;
            lbl_8025D1C0[heap]++;

            return 1;
        }

        idx++;
    }

    *arg2 = NULL;
    return 0;

}

#ifdef NON_MATCHING
s32 xlHeapBlockCacheAdd(int heap, u32 *arg1) {
    s32 i;
    u32 idx;

    if(NUM_WORDS(*arg1) == 0) {
        return 0;
    }

    if(NUM_WORDS(*arg1) < 8) {
        idx = 0;
    } else if(NUM_WORDS(*arg1) < 16) {
        idx = 1;
    } else if(NUM_WORDS(*arg1) < 32) {
        idx = 2;
    } else if(NUM_WORDS(*arg1) < 64) {
        idx = 3;
    } else if(NUM_WORDS(*arg1) < 128) {
        idx = 4;
    } else if(NUM_WORDS(*arg1) < 256) {
        idx = 5;
    } else if(NUM_WORDS(*arg1) < 512) {
        idx = 6;
    } else if(NUM_WORDS(*arg1) < 1024) {
        idx = 7;
    } else if(NUM_WORDS(*arg1) < 4096) {
        idx = 8;
    } else if(NUM_WORDS(*arg1) < 8192) {
        idx = 9;
    } else {
        idx = 10;
    }

    for(i = 0; i < 32; i++) {
        if(gapHeapBlockCache[heap][idx][i] == NULL || NUM_WORDS(*gapHeapBlockCache[heap][idx][i]) < NUM_WORDS(*arg1)) {
            gapHeapBlockCache[heap][idx][i] = arg1;
            return 1;
        }
    }

    return 0;
}
#else
s32 xlHeapBlockCacheAdd(s32 heap, u32* blk);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapBlockCacheAdd.s")
#endif

s32 xlHeapBlockCacheClear(s32 heap, u32 *arg1) {
    s32 idx;
    s32 i;

    if(NUM_WORDS(*arg1) == 0) {
        return 0;
    }

    if(NUM_WORDS(*arg1) < 8) {
        idx = 0;
    } else if(NUM_WORDS(*arg1) < 16) {
        idx = 1;
    } else if(NUM_WORDS(*arg1) < 32) {
        idx = 2;
    } else if(NUM_WORDS(*arg1) < 64) {
        idx = 3;
    } else if(NUM_WORDS(*arg1) < 128) {
        idx = 4;
    } else if(NUM_WORDS(*arg1) < 256) {
        idx = 5;
    } else if(NUM_WORDS(*arg1) < 512) {
        idx = 6;
    } else if(NUM_WORDS(*arg1) < 1024) {
        idx = 7;
    } else if(NUM_WORDS(*arg1) < 4096) {
        idx = 8;
    } else if(NUM_WORDS(*arg1) < 8192) {
        idx = 9;
    } else {
        idx = 10;
    }

    for(i = 0; i < 32; i++) {
        if(gapHeapBlockCache[heap][idx][i] == arg1) {
            gapHeapBlockCache[heap][idx][i] = NULL;
            return 1;
        }
    }

    return 0;
}

s32 xlHeapBlockCacheReset(int heap) {
    int i;
    int j;
    u32 *tmp;

    lbl_8025D1C0[heap] = 0;
    lbl_8025D1C8[heap] = 0;
    gnHeapTakeCacheCount[heap] = 0;

    for(i = 0; i < 11; i++) {
        for(j = 0; j < 32; j++) {
            gapHeapBlockCache[heap][i][j] = NULL;
        }
    }

    tmp = gpHeapBlockFirst[heap];

    while(NUM_WORDS(*tmp)) {
        s32 tmp2 = NUM_WORDS(*tmp);

        if(IS_FREE(*tmp)) {
            xlHeapBlockCacheAdd(heap, tmp);
        }

        tmp += tmp2 + 1;
    }

    return 1;
}

#ifdef NON_MATCHING
s32 xlHeapCompact(s32 heap) {
    s32 r8, r7, r6, r5, r4, r0;
    u32 *cur = gpHeapBlockFirst[heap];
    u32 *next;
    u32 *prev = NULL;
    s32 size;
    u32 next_v;

    while((size = *cur & 0xFFFFFF, size != 0)) {
        next = cur + size + 1;
        next_v = *next;

        if(*cur & 0x1000000) {
            if(r8 < size) {
                r8 = size;
            }

            if(r7 < size) {
                r7 = size;
            }

            if(r6 < size) {
                r6 = size;
            }

            if(r5 < size) {
                r5 = size;
            }

            if(r4 < size) {
                r4 = size;
            }

            if(r0 < size) {
                r0 = size;
            }

            if((next_v & 0xFFFFFF) != 0 && (next_v & 0x1000000)) {
                size += next_v + 1;
                next = &next[(next_v & 0xFFFFFF) + 1];
            }

            if(prev != NULL && *prev & 0x1000000) {
                u32 amt = (*prev & 0xFFFFFF) + size + 1;

                *prev = (amt << 0x1A) | amt | 0x1000000;
            } else {
                *cur = ((*cur & 0xFFFFFF) << 0x1A) | (*cur & 0xFFFFFF) | 0x1000000;
                prev = cur;
            }
        } else {
            prev = cur;
        }

        cur = next;
    }

    xlHeapBlockCacheReset(heap);

    return 1;
}
#else
s32 xlHeapCompact(s32);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapCompact.s")
#endif

#ifdef NON_MATCHING
s32 xlHeapTake(void **dst, size_t size) {
    s32 block_found = 0;
    s32 heap_idx = (size >> 30) & 1;
    s32 align;
    s32 words;
    s32 stack_34;
    u32 *new_block;
    u32 i;
    u32 uVar2;
    u32 uVar5;
    u32 *puVar6;

    *dst = NULL;

    switch(size & 0x30000000) {
        case 0:
            align = 3;
            break;
        case 0x10000000:
            align = 7;
            break;
        case 0x20000000:
            align = 15;
            break;
        case 0x30000000:
            align = 31;
            break;
        default:
            align = 0;
            break;
    }

    words = ((size & 0x8FFFFFFF) + align) >> 2;

    if(words < 1) {
        return 0;
    } else if(words > 0x1000000) {
        return 0;
    }

    i = 1;
    do {
        if(xlHeapBlockCacheGet(heap_idx, words, &new_block, &stack_34)) {
            block_found = 1;
        } else {
            void *last = gpHeapBlockLast[heap_idx];
            new_block = gpHeapBlockFirst[heap_idx];

            while(new_block < last) {
                if(CHKSUM_LO(*new_block) != CHKSUM_HI(*new_block)) {
                    return 0;
                }

                stack_34 = *new_block & 0xFFFFFF;

                if(IS_FREE(*new_block) && stack_34 >= words) {
                    block_found = 1;
                    break;
                }

                new_block += stack_34 + 1;
            }
        }

        if(block_found) {
            if(words == stack_34 - 1) {
                words++;
            }

            if(words < stack_34) {
                uVar2 = new_block[stack_34 + 1];
                uVar5 = (stack_34 - words) - 1;
                puVar6 = new_block + words + 1;

                if(NUM_WORDS(uVar2) && IS_FREE(uVar2)) {
                    xlHeapBlockCacheClear(heap_idx, new_block + stack_34 + 1);
                    uVar5 += NUM_WORDS(uVar2) + 1;
                }

                *puVar6 = uVar5 << 0x1A | uVar5 | 0x1000000;
                xlHeapBlockCacheAdd(heap_idx, puVar6);
            }

            *new_block = (words << 0x1A) | words | 0x2000000;
            gnHeapTakeCacheCount[heap_idx]++;

            for(new_block = new_block + 1; ((u32)new_block & align) != 0; new_block++) {
                *new_block = 0x1234abcd;
            }

            *dst = new_block;
            return 1;
        }

        if(!xlHeapCompact(heap_idx)) {
            return 0;
        }
    } while(i++ < 8);

    return 0;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapTake.s")
#endif

#ifdef NON_MATCHING
s32 xlHeapFree(void **ptr) {
    s32 heap = 0;
    s32 words, next_words;
    u32 *p, *next;

    for(heap = 0; heap < 2; heap++) {
        if(gpHeapBlockFirst[heap] <= *ptr && gpHeapBlockLast[heap] >= *ptr) {
            break;
        }
    }

    if(heap == 2) {
        return 0;
    }

    p = (u32*)*ptr - 1;
    while(*p == 0x1234abcd) {
        p--;
    }

    words = NUM_WORDS(*p);
    if(IS_FREE(*p)) {
        return 0;
    }

    if(!IS_TAKEN(*p)) {
        return 0;
    }

    if(CHKSUM_HI(*p) != CHKSUM_LO(*p)) {
        return 0;
    }

    next = &p[words];
    next_words = NUM_WORDS(*next);
    if(next_words != 0 && IS_FREE(*next)) {
        xlHeapBlockCacheClear(heap, next + 1);
        words += next_words + 1;
    }

    *p = (words << 0x1A) | words | 0x1000000;
    xlHeapBlockCacheAdd(heap, p);
    lbl_8025D1C8[heap]++;
    *ptr = NULL;

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapFree.s")
#endif

s32 xlHeapCopy(void* dst, void* src, int len) {
    if (((u32)src % 4) == 0 && ((u32)dst % 4) == 0) {
        for (; len >= 0x40; len -= 0x40) {
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

        for (; len >= 0x20; len -= 0x20) {
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
            *((u32*)dst)++ = *((u32*)src)++;
        }

        for (; len >= 4; len -= 0x4) {
            *((u32*)dst)++ = *((u32*)src)++;
        }
    }

    for (; len >= 0x10; len -= 0x10) {
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

    for (; len >= 8; len -= 0x8) {
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
        *((u8*)dst)++ = *((u8*)src)++;
    }

    for (; len > 0; len--) {
        *((u8*)dst)++ = *((u8*)src)++;
    }

    return 1;
}

s32 xlHeapFill(u8* src, s32 len, u8 fill) {
    s32 cnt;
    for (cnt = len; cnt > 0x10; cnt -= 0x10) {
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

    for (; cnt > 0; cnt--) {
        *src++ = fill;
    }

    return 1;
}

s32 xlHeapFill32(u32* src, s32 len, u32 fill) {
    s32 cnt;

    for (cnt = len >> 2; cnt > 0x10; cnt -= 0x10) {
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

    for (; cnt > 0; cnt--) {
        *src++ = fill;
    }

    return 1;
}

s32 __xlHeapGetFree(s32 heap, u32* free) {
    u32 total_free;
    u32* hdr_p;
    u32 hdr;
    u32 blk_size;

    if (!xlHeapCompact(heap)) {
        return 0;
    }

    for (hdr_p = gpHeapBlockFirst[heap], total_free = 0; hdr_p < gpHeapBlockLast[heap];) {
        hdr = *hdr_p;
        blk_size = NUM_WORDS(hdr);

        if (CHKSUM_LO(hdr) != CHKSUM_HI(hdr)) {
            return 0;
        }

        if (IS_FREE(hdr)) {
            total_free += blk_size * 4;
        }

        hdr_p += blk_size + 1;
    }
    *free = total_free;
    return 1;
}

s32 xlHeapGetFree(u32* free) {
    return __xlHeapGetFree(0, free);
}

#ifdef NON_MATCHING
s32 xlHeapSetup(void) {
    s32 gpHeap_align[2];
    s32 new_lo[2];
    u32 heap;

    gpHeapOS[0] = OSGetArena1Lo();
    gpHeap_align[0] = ALIGN(gpHeapOS[0], 4);
    new_lo[0] = (u32)OSGetArena1Hi();

    if(new_lo[0] - gpHeap_align[0] > 0x4000000) {
        new_lo[0] = gpHeap_align[0] + 0x4000000;
    }

    OSSetArena1Lo((void*)new_lo[0]);

    gpHeapOS[1] = OSGetArena2Lo();
    gpHeap_align[1] = ALIGN(gpHeapOS[1], 4);
    new_lo[1] = (u32)OSGetArena2Hi();

    if(new_lo[1] - gpHeap_align[1] > 0x4000000) {
        new_lo[1] = gpHeap_align[1] + 0x4000000;
    }

    OSSetArena2Lo((void*)new_lo[1]);

    gnSizeHeap[0] = new_lo[0] - gpHeap_align[0];
    gnSizeHeap[1] = new_lo[1] - gpHeap_align[0];

    gpHeap[0] = gpHeap_align[0];
    gpHeap[1] = gpHeap_align[1];

    for(heap = 0; heap < 2; heap++) {
        s32 uVar1;

        gpHeapBlockFirst[heap] = (void*)gpHeap[heap];
        gpHeapBlockLast[heap] = (u32*)gpHeap[heap] + ((gnSizeHeap[heap] >> 2) - 2);
        *((u32*)gpHeapBlockFirst[heap]) = 0x1000000 | (((gnSizeHeap[heap] >> 2) - 2) << 0x1A) | ((gnSizeHeap[heap] >> 2) - 2);
        *((u32*)gpHeapBlockLast[heap]) = 0;
        xlHeapBlockCacheReset(heap);
    }

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlHeap/xlHeapSetup.s")
#endif

s32 xlHeapReset(void) {
    OSSetArena1Lo(gpHeapOS[0]);
    OSSetArena2Lo(gpHeapOS[1]);
    return 1;
}
