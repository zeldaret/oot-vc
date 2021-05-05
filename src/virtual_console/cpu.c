#include "cpu.h"
#include "rom.h"
#include "xlHeap.h"
#include "system.h"

int cpuGetAddressBuffer(cpu_class_t *cpu, void **buffer, u32 addr);
int cpuFreeCachedAddress(cpu_class_t *cpu, s32, s32);
int func_8003E604(cpu_class_t *cpu, s32, s32);
int rspGetBuffer(void *, void **, u32, void *);


int func_8003D344(cpu_class_t *cpu, u32 *buffer, u32 addr) {
    if(addr >= 0x80000000 && addr < 0xC0000000) {
        *buffer = addr & 0x7FFFFF;
    } else {
        if(cpu->devices[cpu->mem_hi_map[addr >> 0x10]]->unk_0 & 0x100) {
            *buffer = addr + cpu->devices[cpu->mem_hi_map[addr >> 0x10]]->unk_8 & 0x7FFFFF;
        } else {
            return 0;
        }
    }

    return 1;
}

int cpuGetAddressBuffer(cpu_class_t *cpu, void **buffer, u32 addr) {
    cpu_dev_t *dev = cpu->devices[cpu->mem_hi_map[addr >> 0x10]];

    if(dev->unk_4 == gSystem->ram) {
        if(!ramGetBuffer((ram_class_t*)dev->unk_4, buffer, addr + dev->unk_8, NULL)) {
            return 0;
        }
    } else if(dev->unk_4 == gSystem->rom) {
        if(!romGetBuffer(dev->unk_4, buffer, addr + dev->unk_8, NULL)) {
            return 0;
        }
    } else if(dev->unk_4 == gSystem->rsp) {
        if(!rspGetBuffer(dev->unk_4, buffer, addr + dev->unk_8, NULL)) {
            return 0;
        }
    } else {
        return 0;
    }

    return 1;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8003D47C.s")

int cpuInvalidateCache(cpu_class_t *cpu, u32 param_2, s32 param_3) {
    if((param_2 & 0xF0000000) == 0xA0000000) {
        return 1;
    }

    if(!cpuFreeCachedAddress(cpu, param_2, param_3)) {
        return 0;
    }

    func_8003E604(cpu, param_2, param_3);
    return 1;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuGetFunctionChecksum.s")

#ifdef NON_MATCHING
int cpuHeapTake(char **code, cpu_class_t *cpu, recomp_node_t *func, int size) {
    s32 smblk_needed = (size + (0x200 - 1)) / 0x200;
    s32 lgblk_needed = (size + (0xA00 - 1)) / 0xA00;
    int type_swapped = 0;
    int chunk_found = 0;
    int blk_cnt;
    u32 *blks;
    s32 blks_needed;
    int i;
    int j;

    while(!type_swapped) {
        if(func->alloc_type == -1) {
            if(size > 0x3200) {
                func->alloc_type = 2;
            } else {
                func->alloc_type = 1;
            }
        } else if(func->alloc_type == 1) {
            func->alloc_type = 2;
            type_swapped = 1;
        } else if(func->alloc_type == 2) {
            func->alloc_type = 1;
            type_swapped = 1;
        }

        if(func->alloc_type == 1) {
            func->alloc_type = 1;
            blk_cnt = sizeof(cpu->sm_blk_status) / sizeof(*cpu->sm_blk_status);
            blks = cpu->sm_blk_status;
            blks_needed = smblk_needed;
            if(type_swapped && smblk_needed >= 32) {
                func->alloc_type = 3;
                func->unk_0x34 = 0xFFFFFFFF;
                return !!xlHeapTake((void**)code, size);
            }
        } else if(func->alloc_type == 2) {
            func->alloc_type = 2;
            blk_cnt = sizeof(cpu->lg_blk_status) / sizeof(*cpu->lg_blk_status);
            blks = cpu->lg_blk_status;
            blks_needed = lgblk_needed;
        }

        if(blks_needed >= 32) {
            func->alloc_type = 3;
            func->unk_0x34 = 0xFFFFFFFF;
            return !!xlHeapTake((void**)code, size);
        }
        
        for(i = 0; i != blk_cnt; i++) {
            u32 mask = (1 << blks_needed) - 1;

            if(blks[i] != 0xFFFFFFFF) {
                for(j = 0; j < 33 - blks_needed; j++) {
                    if((blks[i] & mask) == 0) {
                        chunk_found = 1;
                        blks[i] |= mask;
                        func->unk_0x34 = (blks_needed << 0x10) | ((i * 32) + j);
                        break;
                    }

                    mask <<= 1;
                }

                if(chunk_found) {
                    break;
                }
            }
        }

        if(chunk_found) {
            if(func->alloc_type == 1) {
                *code = (char*)cpu->sm_blk_code + ((func->unk_0x34 & 0xFFFF) * 0x200);
            } else {
                *code = (char*)cpu->lg_blk_code + ((func->unk_0x34 & 0xFFFF) * 0xA00);
            }

            return 1;
        }

    }

    if(type_swapped) {
        func->alloc_type = -1;
        func->unk_0x34 = 0xFFFFFFFF;
    }

    return 0;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuHeapTake.s")
#endif

int cpuHeapFree(cpu_class_t *cpu, recomp_node_t *func) {
    u32 mask;
    u32 blk_idx;
    u32 *blk;
    
    if(func->alloc_type == 1) {
        blk = cpu->sm_blk_status;
    } else if(func->alloc_type == 2) {
        blk = cpu->lg_blk_status;
    } else {
        if(func->unk_0x00 != 0) {
            if(!xlHeapFree((void**)&func->unk_0x00)) {
                return 0;
            }
        } else {
            if(!xlHeapFree((void**)&func->recompiled_func)) {
                return 0;
            }
        }

        return 1;
    }

    if(func->unk_0x34 == -1) {
        return 0;
    }

    mask = ((1 << (func->unk_0x34 >> 0x10)) - 1) << (func->unk_0x34 & 0x1F);
    blk_idx = (func->unk_0x34 >> 5) & 0x7FF;
    
    if((blk[blk_idx] & mask) == mask) {
        blk[blk_idx] &= ~mask;
        func->alloc_type = -1;
        func->unk_0x34 = -1;
        return 1;
    }

    return 0;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuTreeTake.s")