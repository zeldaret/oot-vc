#include "cpu.h"
#include "mips.h"
#include "rom.h"
#include "xlHeap.h"
#include "system.h"
#include "stddef.h"
#include "math.h"
#include "cache.h"
#include "os.h"

s32 func_8003E604(cpu_class_t *cpu, s32, s32);
s32 rspGetBuffer(void *, void **, u32, void *);
s32 cpuEvent(void *obj, s32 event, void *arg);
s32 cpuFindFunction(cpu_class_t *cpu, u32 addr, recomp_node_t **out_node);
s32 libraryTestFunction(void *library, recomp_node_t *node);
void invalidInst();
s32 rspUpdate(void *, s32);
s32 videoForceRetrace(void *);
void *VISetPostRetraceCallback(void*);

extern char cpu_class_name[];
extern void (*lbl_8025CFE8)();
extern u32 *cpuCompile_DSLLV_function;
extern u32 *cpuCompile_DSRLV_function;
extern u32 *cpuCompile_DSRAV_function;
extern u32 *cpuCompile_DMULT_function;
extern u32 *cpuCompile_DMULTU_function;
extern u32 *cpuCompile_DDIV_function;
extern u32 *cpuCompile_DDIVU_function;
extern u32 *cpuCompile_DADD_function;
extern u32 *cpuCompile_DADDU_function;
extern u32 *cpuCompile_DSUB_function;
extern u32 *cpuCompile_DSUBU_function;
extern u32 *cpuCompile_S_SQRT_function;
extern u32 *cpuCompile_D_SQRT_function;
extern u32 *cpuCompile_W_CVT_SD_function;
extern u32 *cpuCompile_L_CVT_SD_function;
extern u32 *cpuCompile_CEIL_W_function;
extern u32 *cpuCompile_FLOOR_W_function;
extern u32 *cpuCompile_ROUND_W_function;
extern u32 *cpuCompile_TRUNC_W_function;
extern u32 *cpuCompile_LB_function;
extern u32 *cpuCompile_LH_function;
extern u32 *cpuCompile_LW_function;
extern u32 *cpuCompile_LBU_function;
extern u32 *cpuCompile_LHU_function;
extern u32 *cpuCompile_SB_function;
extern u32 *cpuCompile_SH_function;
extern u32 *cpuCompile_SW_function;
extern u32 *cpuCompile_LDC_function;
extern u32 *cpuCompile_SDC_function;
extern u32 *cpuCompile_LWL_function;
extern u32 *cpuCompile_LWR_function;

u64 get_cp0_reg_mask[] = { 
    0x000000008000003F, 0x000000000000003F, 0x000000003FFFFFFF, 0x000000003FFFFFFF,
    0xFFFFFFFFFFFFFFF0, 0x0000000001FFE000, 0x000000000000001F, 0x0000000000000000,
    0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFE0FF, 0x00000000FFFFFFFF,
    0x00000000FFFFFFFF, 0x00000000F000FF7C, 0xFFFFFFFFFFFFFFFF, 0x000000000000FFFF,
    0x00000000FFFFEFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFB, 0x000000000000000F,
    0x00000000FFFFFFF0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000, 0x00000000000000FF, 0x00000000FFBFFFFF,
    0x00000000FFFFFFFF, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000
};

u64 set_cp0_reg_mask[] = {
    0x000000000000003F, 0x000000000000003F, 0x000000003FFFFFFF, 0x000000003FFFFFFF,
    0xFFFFFFFFFFFFFFF0, 0x0000000001FFE000, 0x000000000000001F, 0x0000000000000000,
    0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFE0FF, 0x00000000FFFFFFFF,
    0x00000000FFFFFFFF, 0x0000000000000300, 0xFFFFFFFFFFFFFFFF, 0x000000000000FFFF,
    0x00000000FFFFEFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFB, 0x000000000000000F,
    0x00000000FFFFFFF0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000, 0x00000000000000FF, 0x00000000FFBFFFFF,
    0x00000000FFFFFFFF, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000
};

u8 lbl_80170980[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
};

u8 lbl_801709C0[] = {
    1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1,
    1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1,
};

u8 lbl_80170A00[] = {
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

u32 lbl_80170A20[] = {
    0x8F480018, 0x11000014, 0x00000000, 0x4448F800, 0x00000000
};

u32 lbl_80170A20_2[] = {
    0x8CBB0018, 0x1360000A, 0x00000000, 0x445BF800, 0x00000000
};

u32 lbl_80170A20_3[] = {
    0x13600009, 0xACBF011C, 0x445BF800
};

u32 lbl_80170A20_4[] = {
    0x8F5B0018, 0x13600013, 0x00000000, 0x8F5B012C, 0x44DBF800
};

s32 cpuHackHandler(cpu_class_t *cpu) {
    s32 tmp1 = 0;
    s32 tmp2 = 0;
    s32 tmp3 = 0;
    u32 *buf;
    s32 ram_size;
    s32 i;

    if(xlObjectTest(gSystem->ram, &gClassRAM) && ramGetBuffer(gSystem->ram, (void**)&buf, 0, NULL)) {
        if(!ramGetSize(gSystem->ram, &ram_size)) {
            return 0;
        }

        for(i = 0; i < ram_size >> 2 && (tmp1 != -1 || tmp2 != -1 || tmp3 != -1); i++) {
            if(tmp1 != -1) {
                if(buf[i] == lbl_80170A20[tmp1]) {
                    tmp1++;
                    if(tmp1 == 5U) {
                        buf[i - 3] = 0;
                        tmp1 = -1;
                    }
                } else {
                    tmp1 = 0;
                }
            }

            if(tmp2 != -1) {
                if(buf[i] == lbl_80170A20_2[tmp2]) {
                    tmp2++;
                    if(tmp2 == 5U) {
                        buf[i - 3] = 0;
                        tmp2 = -1;
                    }
                } else if(buf[i] == lbl_80170A20_3[tmp2]) {
                    tmp2++;
                    if(tmp2 == 3U) {
                        buf[i - 2] = 0;
                        tmp2 = -1;
                    }
                } else {
                    tmp2 = 0;
                }
            }

            if(tmp3 != -1) {
                if(buf[i] == lbl_80170A20_4[tmp3]) {
                    tmp3++;
                    if(tmp3 == 5U) {
                        buf[i - 3] = 0;
                        tmp3 = -1;
                    }
                } else {
                    tmp3 = 0;
                }
            }
        }
    }

    return tmp1 == -1 && tmp2 == -1 && tmp3 == -1;
}

s32 cpuFreeCachedAddress(cpu_class_t *cpu, s32 addr_start, s32 addr_end) {
    s32 i;
    s32 j;
    recomp_cache_t *recomp_cache = cpu->recomp_cache;

    for(i = 0;i < cpu->cache_cnt;) {
        if(addr_start <= recomp_cache[i].n64_addr && recomp_cache[i].n64_addr <= addr_end) {
            for(j = i; j < cpu->cache_cnt - 1; j++) {
                recomp_cache[j] = recomp_cache[j + 1];
            }
            cpu->cache_cnt--;
        } else {
            i++;
        }
    }

    return 1;
}

s32 cpuFindCachedAddress(cpu_class_t *cpu, s32 addr, u32 **code) {
    s32 i;
    s32 j;
    recomp_cache_t *cache = cpu->recomp_cache;

    for(i = 0; i < cpu->cache_cnt; i++) {
        if(addr == cache[i].n64_addr) {
            
            if(i > 128) {
                recomp_cache_t found = cache[i];
                while(i > 0) {
                    cache[i] = cache[i - 1];
                    i--;
                }
                cache[i] = found;
            }

            if(cache[i].node->unk_0x28 > 0) {
                cache[i].node->unk_0x28 = cpu->call_cnt;
            }

            *code = cache[i].recomp_addr;
            return 1;
        }
    }

    return 0;
}

#ifdef NON_MATCHING
s32 cpuTestInterrupt(cpu_class_t *cpu, s32 intr) {
    cpu->cp0[CP0_CAUSE].sd |= ((intr & 0xFF) << 8);

    if(cpu->cp0[CP0_STATUS].d & 6) {
        return 0;
    }

    if(!(cpu->cp0[CP0_STATUS].d & 1)) {
        return 0;
    }

    if(((cpu->cp0[CP0_STATUS].sd & 0xFF00) >> 8) & (intr & 0xFF)) {
        return 1;
    }

    return 0;
}
#else
s32 cpuTestInterrupt(cpu_class_t *cpu, s32 intr);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuTestInterrupt.s")
#endif

#ifdef NON_MATCHING
inline s32 cpuTestInterruptInl(cpu_class_t *cpu, s32 intr) {
    cpu->cp0[CP0_CAUSE].sd |= ((intr & 0xFF) << 8);

    if(cpu->cp0[CP0_STATUS].d & 6) {
        return 0;
    }

    if(!(cpu->cp0[CP0_STATUS].d & 1)) {
        return 0;
    }

    if(((cpu->cp0[CP0_STATUS].sd & 0xFF00) >> 8) & (intr & 0xFF)) {
        return 1;
    }

    return 0;
}

s32 cpuException(cpu_class_t *cpu, s32 ex, u32 intr) {
    if(cpu->cp0[CP0_STATUS].w[1] & (STATUS_ERL | STATUS_EXL)) {
        return 0;
    }

    if(ex == CP0_EX_NONE) {
        return 0;
    }

    if((ex >= CP0_EX_RFU16 && ex <= CP0_EX_RFU22) || (ex >= CP0_EX_RFU24 && ex <= CP0_EX_RFU30)) {
        return 0;
    }

    if(ex == CP0_EX_RSVD_INS) {
        return 0;
    }

    if(ex == CP0_EX_INTERRUPT) {
        if(!cpuTestInterruptInl(cpu, intr)) {
            return 0;
        }
    } else {
        cpu->pc -= 4;
        cpu->status |= 4;
    }

    cpu->status &= ~8;
    if(!(cpu->status & 0x10)) {
        cpuHackHandler(cpu);
        cpu->status |= 0x10;
    }

    if(cpu->unk_0x24 != -1) {
        cpu->unk_0x24 = -1;
        cpu->cp0[CP0_EPC].d = cpu->pc - 4;
        cpu->cp0[CP0_CAUSE].d |= CAUSE_BD;
    } else {
        cpu->cp0[CP0_EPC].d = cpu->pc;
    }

    cpu->status &= ~0x80;
    cpu->cp0[CP0_STATUS].w[1] |= 2;
    cpu->cp0[CP0_CAUSE].sd = (cpu->cp0[CP0_CAUSE].sd & ~0x7C) | (ex << 2);

    if(ex < 4) {
        cpu->pc = 0x80000000;
    } else {
        cpu->pc = 0x80000180;
    }

    cpu->status |= 0x24;

    if(!func_8005D614(gSystem->unk_0x0058, cpu, -1)) {
        return 0;
    }

    return 1;
}
#else
s32 cpuException(cpu_class_t *cpu, s32 ex, u32 intr);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuException.s")
#endif

/**
 * @brief Creates a new device and registers memory space for that device.
 * 
 * @param cpu The emulated VR4300.
 * @param new_dev_idx A pointer to the index in the cpu->devices array which the device was created.
 * @param dev_obj The object which will handle reuqests for this device.
 * @param vaddr Starting address of the device's address space.
 * @param paddr Starting physical address of the device's address space.
 * @param size Size of the device's memory space.
 * @param create_arg An argument which will be passed back to the device's event handler.
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuMakeDevice(cpu_class_t *cpu, u32 *new_dev_idx, void *dev_obj, u32 vaddr, u32 paddr, u32 size, u32 create_arg) {
    s32 i;
    s32 j;
    cpu_dev_t *new_dev;
    u32 addr;

    for(i = ((create_arg >> 8) & 1) ? 128 : 0; i < 256; i++) {
        if(cpu->devices[i] == NULL) {
            if (new_dev_idx != NULL) {
                *new_dev_idx = i;
            }

            if(!xlHeapTake((void**)&new_dev, 0x40)) {
                return 0;
            }

            cpu->devices[i] = new_dev;
            new_dev->create_arg = create_arg;
            new_dev->dev_obj = dev_obj;
            new_dev->addr_offset = paddr - vaddr;
            if(size == 0) {
                new_dev->paddr_start = 0;
                new_dev->vaddr_start = 0;
                new_dev->paddr_end = 0xFFFFFFFF;
                new_dev->vaddr_end = 0xFFFFFFFF;
                for(j = 0; j < 0x10000; j++) {
                    cpu->mem_hi_map[j] = i;
                }
            } else {
                new_dev->vaddr_start = vaddr;
                new_dev->vaddr_end = vaddr + size - 1;
                new_dev->paddr_start = paddr;
                new_dev->paddr_end = paddr + size - 1;

                for(j = size; j > 0; vaddr += 0x10000, j -= 0x10000) {
                    cpu->mem_hi_map[vaddr >> 16] = i;
                }
            }

            return !!xlObjectEvent(dev_obj, 0x1002, (void*)new_dev);
        }
    }

    return 0;
}

s32 cpuCreateTLBDevice(cpu_class_t *cpu, s32 *new_dev_idx, u32 vaddr, u32 paddr, u32 page_size) {
    s32 dev;
    u32 new_dev;

    for(dev = 0x80; dev < 256; dev++) {
        if(dev != cpu->mem_dev_idx) {
            if(cpu->devices[dev] != NULL) {
                if(cpu->devices[dev]->paddr_start <= paddr && paddr <= cpu->devices[dev]->paddr_end) {
                    break;
                }
            }
        }
    }
    if(dev == 256) {
        dev = cpu->mem_dev_idx;
    }

    if(!cpuMakeDevice(cpu, &new_dev, cpu->devices[dev]->dev_obj, vaddr, paddr, page_size, cpu->devices[dev]->create_arg)) {
        return 0;
    }

    if(new_dev_idx != NULL) {
        *new_dev_idx = new_dev;
    }

    return 1;
}

#ifdef NON_MATCHING
s32 cpuSetTLB(cpu_class_t *cpu, s32 index) {
    cpu_tlb_t *tlb = &cpu->tlb[index];
    s32 i;
    s32 page_size;
    s32 tmp;
    u32 vpn2;
    u32 pfn;

    tlb->page_mask.d = cpu->cp0[CP0_PAGEMASK].d & TLB_PGSZ_MASK;
    tlb->entry_hi.d = cpu->cp0[CP0_ENTRYHI].d;
    tlb->entry_lo0.d = cpu->cp0[CP0_ENTRYLO0].d;
    tlb->entry_lo1.d = cpu->cp0[CP0_ENTRYLO1].d;
    if(tlb->entry_lo0.w[1] & TLB_LO_VALD || tlb->entry_lo1.w[1] & TLB_LO_VALD) {
        // page size
        switch(tlb->page_mask.d) {
            case TLB_PGSZ_4K:
                page_size = 4 * 1024;
                break;
            case TLB_PGSZ_16K:
                page_size = 16 * 1024;
                break;
            case TLB_PGSZ_64K:
                page_size = 64 * 1024;
                break;
            case TLB_PGSZ_256K:
                page_size = 256 * 1024;
                break;
            case TLB_PGSZ_1M:
                page_size = 1 * 1024 * 1024;
                break;
            case TLB_PGSZ_4M:
                page_size = 4 * 1024 * 1024;
                break;
            case TLB_PGSZ_16M:
                page_size = 16 * 1024 * 1024;
                break;
            default:
                page_size = 0;
                break;
        }

        vpn2 = tlb->entry_hi.w[1] & 0xFFFFE000;
        if(tlb->entry_lo0.w[1] & 2) {
            tmp = (tlb->dev_status.w[1] >> (0 * 16)) & 0xFF;

            if(!xlHeapFree((void**)&cpu->devices[tmp])) {
                return 0;
            }

            cpu->devices[tmp] = NULL;

            for(i = 0; i < 0x10000; i++) {
                if(cpu->mem_hi_map[i] == tmp) {
                    cpu->mem_hi_map[i] = cpu->mem_dev_idx;
                }
            }

            if(!cpuCreateTLBDevice(cpu, &tmp, vpn2, pfn, page_size)) {
                return 0;
            }

            tlb->dev_status.sd = (tmp & 0xFF) | (tlb->dev_status.sd & ~0xFF);
        }

        if(tlb->entry_lo1.w[1] & 2) {
            pfn = (tlb->entry_lo1.w[1] & 0x3FFFFC0) << 6;
            tmp = (tlb->dev_status.w[1] >> 16) & 0xFF;

            if(!xlHeapFree((void**)&cpu->devices[tmp])) {
                return 0;
            }

            cpu->devices[tmp] = NULL;

            for(i = 0; i < 0x10000; i++) {
                if(cpu->mem_hi_map[i] == tmp) {
                    cpu->mem_hi_map[i] = cpu->mem_dev_idx;
                }
            }

            if(!cpuCreateTLBDevice(cpu, &tmp, vpn2 + page_size, pfn, page_size)) {
                return 0;
            }

            tlb->dev_status.sd = ((tmp & 0xFF) << 16) | (tlb->dev_status.sd & ~0xFF00);
        }
    } else {
        tmp = tlb->dev_status.w[1] & 0xFF;

        if(!xlHeapFree((void**)&cpu->devices[tmp])) {
            return 0;
        }

        cpu->devices[tmp] = NULL;

        for(i = 0; i < 0x10000; i++) {
            if(cpu->mem_hi_map[i] == tmp) {
                cpu->mem_hi_map[i] = cpu->mem_dev_idx;
            }
        }

        tmp = (tlb->dev_status.w[1] >> 16) & 0xFF;

        if(!xlHeapFree((void**)&cpu->devices[tmp])) {
            return 0;
        }

        cpu->devices[tmp] = NULL;

        for(i = 0; i < 0x10000; i++) {
            if(cpu->mem_hi_map[i] == tmp) {
                cpu->mem_hi_map[i] = cpu->mem_dev_idx;
            }
        }

        tlb->dev_status.sd = -1;
    }

    return 1;
}
#else
s32 cpuSetTLB(cpu_class_t *cpu, s32 index);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuSetTLB.s")
#endif

#ifdef NON_MATCHING
/**
 * @brief Gets the operating mode of the VR4300
 * 
 * @param status The status bits to determine the mode for.
 * @param mode A pointer to the mode determined.
 * @return s32 1 on success, 0 otherwse.
 */
s32 cpuGetMode(u64 status, u32 *mode) {
    u32 kmode = 2;
    u32 smode = 1;
    u32 umode = 0;
    u32 setmode;

    if(status & STATUS_EXL) {
        *mode = kmode;
        return 1;
    } else if(!(status & STATUS_ERL)) {
        switch(status & 0x18) {
            case 0x10:
                *mode = umode;
                break;
            case 8:
                *mode = smode;
                break;
            case 0:
                *mode = kmode;
                break;
            default:
                return 0;
        }

        return 1;
    }

    return 0;
}
#else
s32 cpuGetMode(u64 status, u32 *mode);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuGetMode.s")
#endif

/**
 * @brief Determines the register size that the VR4300 is using.
 * 
 * @param status Status bits for determining the register size.
 * @param enabled 1 if 64-bits are enabled for registers, 0 for 32-bit registers.
 * @param out_mode The operating mode of the VR4300.
 * @return s32 
 */
s32 cpuGetSize(u64 status, s32 *enabled, s32 *out_mode) {
    u32 mode;
    s32 res;

    *enabled = -1;
    if(out_mode != NULL) {
        *out_mode = -1;
    }

    if(cpuGetMode(status, &mode)) {
        switch(mode) {
            case 0:
                res = 0;
                if(status & 0x20) {
                    res = 1;
                }
                *enabled = res;
                break;
            case 1: 
                res = 0;
                if(status & 0x40) {
                    res = 1;
                }
                *enabled = res;
                break;
            case 2:
                res = 0;
                if(status & 0x80) {
                    res = 1;
                }
                *enabled = res;
                break;
            default:
                return 0;
        }

        if(out_mode != NULL) {
            *out_mode = mode;
        }

        return 1;
    }

    return 0;
}

/**
 * @brief Sets the status bits of the VR4300
 * 
 * @param cpu The emulated VR4300
 * @param mask Unused mask bits for the status register.
 * @param status New status.
 * @param arg3 Unused.
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuSetCP0Status(cpu_class_t *cpu, u32 mask, u64 status, u32 arg3) {
    s32 new_status_mode;
    s32 prev_status_mode;
    s32 new_status_64enabled;
    s32 prev_status_64enabled;

    if(!cpuGetSize(status, &new_status_64enabled, &new_status_mode)) { 
        return 0;
    }

    if(!cpuGetSize(cpu->cp0[CP0_STATUS].d, &prev_status_64enabled, &prev_status_mode)) {
        return 0;
    }

    cpu->cp0[CP0_STATUS].d = status;
    return 1;
}

#ifdef NON_MATCHING
s32 cpuSetRegisterCP0(cpu_class_t *cpu, s32 reg, u64 val) {
    s32 set_reg = 0;

    switch(reg) {
        case CP0_INDEX:
            cpu->cp0[CP0_INDEX].d = (cpu->cp0[CP0_INDEX].d & 0x80000000) | (val & set_cp0_reg_mask[0]);
            break;
        case CP0_RANDOM:
        case 7:
        case CP0_BADVADDR:
        case CP0_PRID:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 27:
        case 31:
            break;
        case CP0_COUNT:
            set_reg = 1;
            break;
        case CP0_COMPARE:
            set_reg = 1;
            xlObjectEvent(gSystem, 0x1001, (void*)3);
            if(cpu->status & 1 || !(val & set_cp0_reg_mask[11])) {
                cpu->status &= ~1;
            } else {
                cpu->status |= 1;
            }

            break;
        case CP0_STATUS:
            cpuSetCP0Status(cpu, set_cp0_reg_mask[12], val & set_cp0_reg_mask[12], 0);
            break;
        case CP0_CAUSE:
            xlObjectEvent(gSystem, val & 0x100 ? 0x1000 : 0x1001, (void*) 0);
            xlObjectEvent(gSystem, val & 0x200 ? 0x1000 : 0x1001, (void*) 0);
            set_reg = 1;
            break;
        case CP0_EPC:
            set_reg = 1;
            break;
        case CP0_CONFIG:
            cpu->cp0[0x10].d = (u32)(val & set_cp0_reg_mask[0x10]);
            break;
        default:
            set_reg = 1;
            break;
    }

    if(set_reg) {
        cpu->cp0[reg].d = val & set_cp0_reg_mask[reg];
    }

    return 1;
}
#else
s32 cpuSetRegisterCP0(cpu_class_t *cpu, s32 reg, u64 val);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuSetRegisterCP0.s")
#endif

#ifdef NON_MATCHING
inline s32 checkflg(cpu_class_t *cpu) {
    s32 i;
    s32 val = 0;

    for(i = 0; i < sizeof(cpu->tlb) / sizeof(*cpu->tlb); i++) {
        if(!(cpu->tlb[i].entry_hi.w[1] & 2)) {
            val += 1;
        }
    }

    return val;
}

s32 cpuGetRegisterCP0(cpu_class_t *cpu, s32 reg, u64 *dest) {
    s32 i;
    s32 res;
    s32 set_reg = 0;

    switch(reg) {
        case 1:
            res = checkflg(cpu);

            *dest = res;
            break;
        case 9:
            set_reg = 1;
            break;
        case 11:
            set_reg = 1;
            break;
        case 14:
            set_reg = 1;
            break;
        case 7: // rsvd?
            *dest = 0;
            break;
        case 8:
            set_reg = 1;
            break;
        case 21:
            *dest = 0;
            break;
        case 22:
            *dest = 0;
            break;
        case 23:
            *dest = 0;
            break;  
        case 24:
            *dest = 0;
            break;
        case 25:
            *dest = 0;
            break;
        case 31:
            *dest = 0;
            break;
        default:
            set_reg = 1;
    }

    if(set_reg) {
        *dest = cpu->cp0[reg].d & get_cp0_reg_mask[reg];
    }

    return 1;
}
#else
s32 cpuGetRegisterCP0(cpu_class_t *cpu, s32 reg, u64 *dest);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuGetRegisterCP0.s")
#endif
#undef NON_MATCHING

/**
 * @brief Mapping of VR4300 to PPC registers.  
 * If bit 0x100 is set the VR4300 register is not directly mapped to any PPC register,
 * Instead the register will use the emulated VR4300 object for saving/loading register values.
 */
u32 reg_map[] = {
    10,     // r0 -> r10
    11,     // at -> r11
    12,     // v0 -> r12
    14,     // v1 -> r14
    15,     // a0 -> r15
    16,     // a1 -> r16
    17,     // a2 -> r17
    18,     // a3 -> r18
    19,     // t0 -> r19
    20,     // t1 -> r20
    21,     // t2 -> r21
    22,     // t3 -> r22
    23,     // t4 -> r23
    24,     // t5 -> r24
    25,     // t6 -> r25
    26,     // t7 -> r26
    0x110,  // s0 -> gpr[16] (no ppc reg)
    0x111,  // s1 -> gpr[17] (no ppc reg)
    0x112,  // s2 -> gpr[18] (no ppc reg)
    0x113,  // s3 -> gpr[19] (no ppc reg)
    0x114,  // s4 -> gpr[20] (no ppc reg)
    0x115,  // s5 -> gpr[21] (no ppc reg)
    0x116,  // s6 -> gpr[22] (no ppc reg)
    0x117,  // s7 -> gpr[23] (no ppc reg)
    27,     // t8 -> r27
    28,     // t9 -> r28
    29,     // k0 -> r29
    30,     // k1 -> r30
    0x11C,  // gp -> gpr[28] (no ppc reg)
    31,     // sp -> r31
    0x11E,  // fp -> gpr[30] (no ppc reg)
    0x11F   // ra -> gpr[31] (no ppc reg)
};

/**
 * @brief Sets CP0 values for returnning from an exception.
 * 
 * @param cpu The emulated VR4300.
 * @return s32 1 on success, 0 otherwise.
 */
s32 __cpuERET(cpu_class_t *cpu) {
    if(cpu->cp0[CP0_STATUS].d & STATUS_ERL) {
        cpu->pc = cpu->cp0[CP0_ERROREPC].d;
        cpu->cp0[CP0_STATUS].d &= ~STATUS_ERL;
    } else {
        cpu->pc = cpu->cp0[CP0_EPC].d;
        cpu->cp0[CP0_STATUS].d &= ~STATUS_EXL;
    }

    cpu->status |= 0x24;
    return 1;
}

/**
 * @brief Sets flags for handling cpu breakpoints.
 * 
 * @param cpu The emulated VR4300.
 * @return s32 1 on success, 0 otherwise.
 */
s32 __cpuBreak(cpu_class_t *cpu) {
    cpu->status |= 2;
    return 1;
}

s32 cpuGetBranchDistance(cpu_class_t *cpu, recomp_node_t *node, s32 *branch_dist, s32 addr, u32 *code) {
    s32 i;

    if(code == NULL) {
        *branch_dist = 0;
        return 1;
    }

    for(i = 0; i < node->branch_cnt; i++) {
        if(addr == node->branches[i].n64_target) {
            *branch_dist = node->branches[i].branch_dist;
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Checks the type of delay an instruction has.
 * 
 * @param inst The instruction to determine the delay type for.
 * @return s32 The type of delay the instruction has.
 */
s32 cpuCheckDelaySlot(u32 inst) {
    s32 ret = 0;

    if(inst == 0) {
        return 0;
    }

    switch(MIPS_OP(inst)) {
        case OPC_SPECIAL:
            switch(SPEC_FUNCT(inst)) {
                case SPEC_JR:
                    ret = 0xD05;
                    break;
                case SPEC_JALR:
                    ret = 0x8AE;
                    break;
            }
            break;
        case OPC_REGIMM:
            switch(REGIMM_SUB(inst)) {
                case REGIMM_BLTZ:
                case REGIMM_BGEZ:
                case REGIMM_BLTZL:
                case REGIMM_BGEZL:
                case REGIMM_BLTZAL:
                case REGIMM_BGEZAL:
                case REGIMM_BLTZALL:
                case REGIMM_BGEZALL:
                    ret = 0x457;
                    break;
            }
            break;
        case OPC_CP0:
            switch(MIPS_CP0FUNC(inst)) {
                default:
                    switch((inst >> 0x15) & 0x1F) {
                        default:
                            break;
                        case 8:
                            switch((inst >> 0x10) & 0x1F) {
                                case 0:
                                case 1:
                                case 2:
                                case 3:
                                    ret = 0x457;
                                    break;
                                default:
                                    break;
                            }
                            break;
                    }
                    break;
                case 1:
                case 2:
                case 5:
                case 8:
                case 0x18:
                    break;
            }
            break;
        case OPC_CP1:
            if(MIPS_FSUB(inst) == 8) {
                switch((inst >> 0x10) & 0x1F) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        ret = 0x457;
                        break;
                    default:
                        break;
                }
                break;
            }
            break;
        case OPC_J:
            ret = 0xD05;
            break;
        case OPC_JAL:
            ret = 0x8AE;
            break;
        case OPC_BEQ:
        case OPC_BNE:
        case OPC_BLEZ:
        case OPC_BGTZ:
        case OPC_BEQL:
        case OPC_BNEL:
        case OPC_BLEZL:
        case OPC_BGTZL:
            ret = 0x457;
            break;
    }

    return ret;
}

/**
 * @brief Filles a code section of NOPs
 * 
 * @param code Pointer to fill nops to.
 * @param pos Position in @code to start filling.
 * @param cnt The amount of NOPs to fill.
 */
void cpuNOPFill(u32 *code, s32 *pos, s32 cnt) {
    if(code == NULL) {
        *pos += cnt;
        return;
    }

    while(cnt-- != 0) {
        code[(*pos)++] = 0x60000000; // NOP
    }
}

#ifdef NON_MATCHING
// gSystem is loaded each time in original
s32 func_8000E734(cpu_class_t *cpu, u32 inst, u32 prev_inst, u32 next_inst, s32 pc, u32 *code, s32 *arg6, s32 *arg7) {
    if(gSystem->rom_id == 'CLBJ' || gSystem->rom_id == 'CLBE' || gSystem->rom_id =='CLBP') {
        // lw sp, 0x0000(a0)
        // lw ra, 0x0004(a0)
        // lw s0, 0x0008(a0)
        if(inst == 0x8C9F0004 && prev_inst == 0x8C9D0000 && next_inst == 0x8C900008) {
            cpu->unk_0x12220 |= 2;
        }
    } else if(gSystem->rom_id == 'NFXJ' || gSystem->rom_id == 'NFXE' || gSystem->rom_id == 'NFXP') {
        // nop
        // lw ra, 0x003C(sp)
        // sw s2, 0x0040(sp)
        if(inst == 0x8FBF003C && prev_inst == 0x00000000 && next_inst == 0xAFB20040) {
            cpu->unk_0x12220 |= 2;
        }
    }

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E734.s")
#endif

#ifdef NON_MATCHING
// Same issue as above, some other
s32 func_8000E81C(cpu_class_t *cpu, u32 inst, u32 prev_inst, u32 next_inst, s32 arg4, u32 *code, s32 *pos, u32 *arg7) {
    if(gSystem->rom_id == CLBJ || gSystem->rom_id == CLBE || gSystem->rom_id ==CLBP) {
        // sw sp, 0x0000(a0)
        // sw ra, 0x0004(a0)
        // sw s0, 0x0008(a0)
        if(inst == 0xAC9F0004 && prev_inst == 0xAC9D0000 && next_inst == 0xAC900008) {
            if(code != NULL) {
                code[(*pos)++] = 0x80A30000 + ((u32)&cpu->n64_ra - (u32)cpu); // addic r0, r0, 0xA380
            } else {
                (*pos)++;
            }
            *arg7 = reg_map[31];
            if(*arg7 & 0x100) {
                if(code != NULL) {
                    code[(*pos)++] = 0x90A30000 + (u16)((u32)&cpu->gpr[31] - (u32)cpu + 4);
                } else {
                    (*pos)++;
                }
            } else {
                if(code != NULL) {
                    code[(*pos)++] = 0x7CA02B78 | (*arg7 << 16);
                } else {
                    (*pos)++;
                }
            }
            cpu->unk_0x12220 |= 2;
        }
    } else if(gSystem->rom_id == NFXJ || gSystem->rom_id == NFXE || gSystem->rom_id == NFXP) {
        // move s4, a0
        // sw ra, 0x003C(sp)
        // sw s0, 0x0018(sp)
        if(inst == 0xAFBF003C && prev_inst == 0x0080A025 && next_inst == 0xAFB00018) {
            if(code != NULL) {
                // addic r0, r0, offset(cpu_class_t, n64_ra)
                code[(*pos)++] = 0x80A30000 + ((u32)&cpu->n64_ra - (u32)cpu);
            } else {
                (*pos)++;
            }

            *arg7 = reg_map[31];
            if(*arg7 & 0x100) {
                if(code != NULL) {
                    // stw r5, offsetof(cpu_class_t, gpr[31]) + 4(r3)
                    code[(*pos)++] = 0x90A30000 + (u16)((u32)&cpu->gpr[31] - (u32)cpu + 4);
                } else {
                    (*pos)++;
                }
            } else {
                if(code != NULL) {
                    // mr *arg7, r5
                    code[(*pos)++] = 0x7CA02B78 | (*arg7 << 16);
                } else {
                    (*pos)++;
                }
            }
            cpu->unk_0x12220 |= 2;
        }
    }

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E81C.s")
#endif

#ifdef NON_MATCHING
/**
 * @brief The main MIPS->PPC Dynamic recompiler.
 * Largely unfinished.
 * @param cpu The emulated VR4300.
 * @param inst_addr The address to recompile.
 * @param node The function that is being recompiled.
 * @param code Pointer to the recompiled code.
 * @param pos Pointer to the current position in the recompiled code.
 * @param delay 1 if we are recompiling a delay slot.
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuGetPPC(cpu_class_t *cpu, s32 *inst_addr, recomp_node_t *node, u32 *code, s32 *pos, s32 delay) {
    u32 *mips_buf;
    u32 prev_inst;
    u32 cur_inst;
    u32 next_inst;
    s32 skip;
    s32 cur_inst_addr;
    s32 i;
    s32 bVar2;
    u32 offset;
    s32 iVar5 = 0;
    u32 rA;
    u32 rS;
    u32 rD;
    u32 rB;
    s32 prev_pos;


    if(!cpuGetAddressBuffer(cpu, (void**)&mips_buf, *inst_addr)) {
        return 0;
    }

    skip = 0;
    next_inst = mips_buf[1];
    cur_inst = mips_buf[0];
    prev_inst = mips_buf[-1];
    cur_inst_addr = *inst_addr;
    
    (*inst_addr) += 4;
    
    for(i = 0; i < cpu->hack_cnt; i++) {
        if(cur_inst_addr == cpu->hacks[i].addr && cur_inst == cpu->hacks[i].expected) {
            if(cpu->hacks[i].replacement == -1) {
                skip = 1;
            }

            if(cpu->hacks[i].replacement != -1) {
                cur_inst = cpu->hacks[i].replacement;
            }
        }

        if((cur_inst_addr + 4) == cpu->hacks[i].addr && next_inst == cpu->hacks[i].expected) {
            if(cpu->hacks[i].replacement != -1) {
                next_inst = cpu->hacks[i].replacement;
            }
        }

        if((cur_inst_addr - 4) == cpu->hacks[i].addr && prev_inst == cpu->hacks[i].expected) {
            if(cpu->hacks[i].replacement != -1) {
                prev_inst = cpu->hacks[i].replacement;
            }
        }
    }

    cpu->unk_0x12288 = 0;
    if(*pos == 0) {
        cpu->unk_0x1228C = -1;

        if(!skip) {
            if(!cpuCheckDelaySlot(cur_inst)) {
                if((node->n64_end_addr - node->n64_start_addr) / 4 >= 25) {
                    skip = 1;
                }

                if((node->n64_end_addr - node->n64_start_addr) / 4 <= 26) {
                    bVar2 = 1;
                }
            }

            if(code != NULL) {
                code[(*pos)++] = 0x3CA00000 | ((u32)node >> 0x10);
                code[(*pos)++] = 0x60A50000 | ((u32)node & 0xFFFF);
                code[(*pos)++] = 0x80C50028;
                code[(*pos)++] = 0x2C060000;
                code[(*pos)++] = 0x41820008;
                code[(*pos)++] = 0x90850028;
            }

            if(bVar2) {
                if(code != NULL) {
                    code[(*pos)++] = 0x80a3003c;
                    code[(*pos)++] = 0x7ca62850;
                    code[(*pos)++] = 0x2c050002;
                    code[(*pos)++] = 0x41800014;
                    code[(*pos)++] = 0x3ca00000 | (cur_inst_addr >> 0x10);
                    code[(*pos)++] = 0x7ca62850 | (cur_inst_addr & 0xFFFF);
                    offset = ((u32)cpu->execute_opcode -(u32)&code[*pos]);
                    code[(*pos)++] = 0x48000001 | (offset & 0x3FFFFFC);
                } else {
                    (*pos) += 16;
                }
            }
        }
    } else {
        iVar5 = cpuCheckDelaySlot(prev_inst);
    }

    bVar2 = 0;
    if(cur_inst != 0 && (iVar5 || delay)) {
        bVar2 = 1;
    }

    if(!skip) {
        if(code == NULL || bVar2) {
            cpu->unk_0x12274 = 0;
            cpu->unk_0x12220 = 0;
            cpu->unk_0x1228C = -1;
            cpu->unk_0x12294 = 0;
        }

        switch(MIPS_OP(cur_inst)) {
            case OPC_SPECIAL:
                switch(SPEC_FUNCT(cur_inst)) {
                    case SPEC_SLL:
                        if(cur_inst == 0) {
                            if(code != NULL) {
                                code[(*pos)++] = 0x60000000; // NOP
                            }
                        }

                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RD(cur_inst)];

                        if(rA & 0x100) {
                            rA = 5;
                        }

                        rS = reg_map[MIPS_RT(cur_inst)];
                        if(rS & 0x100) {
                            rS = 6;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RT(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rt]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        if(code != NULL) {
                            // li r7, shift_amt
                            code[*pos] = 0x38E00000 | ((cur_inst >> 6) & 0x1F);
                        }

                        (*pos)++;

                        if(code != NULL) {
                            // slw rA, rS, r7
                            code[*pos] = 0x7C000030 | (rS << 21) | (rA << 16) | 0x3800;
                        }

                        (*pos)++;

                        if(reg_map[MIPS_RD(cur_inst)] & 0x100) {
                            cpu->unk_0x12274 = 2;
                            cpu->tmp_mips_dest_reg = MIPS_RD(cur_inst);
                            cpu->tmp_ppc_dest_reg = rA;
                            if(code != NULL) {
                                // stw r5, cpu->gpr[rd]
                                code[*pos] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                            }
                        }

                        break;
                    case SPEC_SRL:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RD(cur_inst)];

                        if(rA & 0x100) {
                            rA = 5;
                        }

                        rS = reg_map[MIPS_RT(cur_inst)];
                        if(rS & 0x100) {
                            rS = 6;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RT(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rt]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        if(code != NULL) {
                            // li r7, shift_amt
                            code[*pos] = 0x38E00000 | ((cur_inst >> 6) & 0x1F);
                        }

                        (*pos)++;

                        if(code != NULL) {
                            // srw rA, rS, r7
                            code[*pos] = 0x7C000430 | (rS << 21) | (rA << 16) | 0x3800;
                        }

                        (*pos)++;

                        if(reg_map[MIPS_RD(cur_inst)] & 0x100) {
                            cpu->unk_0x12274 = 2;
                            cpu->tmp_mips_dest_reg = MIPS_RD(cur_inst);
                            cpu->tmp_ppc_dest_reg = rA;
                            if(code != NULL) {
                                // stw r5, cpu->gpr[rd]
                                code[*pos] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                            }
                        }

                        break;
                    case SPEC_SRA:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RD(cur_inst)];

                        if(rA & 0x100) {
                            rA = 5;
                        }

                        rS = reg_map[MIPS_RT(cur_inst)];
                        if(rS & 0x100) {
                            rS = 6;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RT(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rt]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        if(code != NULL) {
                            // srawi rA, rS, r7
                            code[*pos] = 0x7C000670 | (rS << 21) | (rA << 16) | (((cur_inst >> 6) & 0x1F) << 11);
                        }

                        (*pos)++;

                        if(reg_map[MIPS_RD(cur_inst)] & 0x100) {
                            cpu->unk_0x12274 = 2;
                            cpu->tmp_mips_dest_reg = MIPS_RD(cur_inst);
                            cpu->tmp_ppc_dest_reg = rA;
                            if(code != NULL) {
                                // stw r5, cpu->gpr[rd]
                                code[*pos] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                            }
                        }

                        break;
                    case SPEC_SLLV:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RD(cur_inst)];

                        if(rA & 0x100) {
                            rA = 5;
                        }

                        rS = reg_map[MIPS_RT(cur_inst)];
                        if(rS & 0x100) {
                            rS = 6;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RT(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rt]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        rB = reg_map[MIPS_RS(cur_inst)];
                        if(rB & 0x100) {
                            rB = 7;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RS(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rs]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        if(code != NULL) {
                            // andi. rB, rB, 0x1F
                            code[*pos] = 0x70000000 | (rB << 21) | (rB << 16) | 0x1F;
                        }

                        (*pos)++;

                        if(code != NULL) {
                            // slw rA, rS, rB
                            code[*pos] = 0x7C000030 | (rS << 21) | (rA << 16) | (rB << 11);
                        }

                        (*pos)++;

                        if(reg_map[MIPS_RD(cur_inst)] & 0x100) {
                            cpu->unk_0x12274 = 2;
                            cpu->tmp_mips_dest_reg = MIPS_RD(cur_inst);
                            cpu->tmp_ppc_dest_reg = rA;
                            if(code != NULL) {
                                // stw r5, cpu->gpr[rd]
                                code[*pos] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                            }
                        }

                        break;
                    case SPEC_SRLV:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RD(cur_inst)];

                        if(rA & 0x100) {
                            rA = 5;
                        }

                        rS = reg_map[MIPS_RT(cur_inst)];
                        if(rS & 0x100) {
                            rS = 6;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RT(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rt]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        rB = reg_map[MIPS_RS(cur_inst)];
                        if(rB & 0x100) {
                            rB = 7;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RS(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rs]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        if(code != NULL) {
                            // andi. rB, rB, 0x1F
                            code[*pos] = 0x70000000 | (rB << 21) | (rB << 16) | 0x1F;
                        }

                        (*pos)++;

                        if(code != NULL) {
                            // srw rA, rS, rB
                            code[*pos] = 0x7C000430 | (rS << 21) | (rA << 16) | (rB << 11);
                        }

                        (*pos)++;

                        if(reg_map[MIPS_RD(cur_inst)] & 0x100) {
                            cpu->unk_0x12274 = 2;
                            cpu->tmp_mips_dest_reg = MIPS_RD(cur_inst);
                            cpu->tmp_ppc_dest_reg = rA;
                            if(code != NULL) {
                                // stw r5, cpu->gpr[rd]
                                code[*pos] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                            }
                        }

                        break;
                    case SPEC_SRAV:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RD(cur_inst)];

                        if(rA & 0x100) {
                            rA = 5;
                        }

                        rS = reg_map[MIPS_RT(cur_inst)];
                        if(rS & 0x100) {
                            rS = 6;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RT(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rt]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        rB = reg_map[MIPS_RS(cur_inst)];
                        if(rB & 0x100) {
                            rB = 7;
                            if(cpuIsBranchTarget(cpu, cur_inst_addr, MIPS_RS(cur_inst))) {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // mr r6, cpu->tmp_ppc_dest_reg
                                    code[(*pos)++] = 0x7C060378 | (cpu->tmp_ppc_dest_reg << 0xB) | (cpu->tmp_ppc_dest_reg << 0x15);
                                }
                            } else {
                                if(code == NULL) {
                                    (*pos)++;
                                } else {
                                    // lwz r6, cpu->gpr[rs]
                                    code[(*pos)++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                                }
                            }
                        }

                        if(code != NULL) {
                            // andi. rB, rB, 0x1F
                            code[*pos] = 0x70000000 | (rB << 21) | (rB << 16) | 0x1F;
                        }

                        (*pos)++;

                        if(code != NULL) {
                            // sraw rA, rS, rB
                            code[*pos] = 0x7C000630 | (rS << 21) | (rA << 16) | (rB << 11);
                        }

                        (*pos)++;

                        if(reg_map[MIPS_RD(cur_inst)] & 0x100) {
                            cpu->unk_0x12274 = 2;
                            cpu->tmp_mips_dest_reg = MIPS_RD(cur_inst);
                            cpu->tmp_ppc_dest_reg = rA;
                            if(code != NULL) {
                                // stw r5, cpu->gpr[rd]
                                code[*pos] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                            }
                        }

                        break;

                    case SPEC_JR:
                        // Compile delay slot if not a NOP
                        if(next_inst != 0) {
                            if(!cpuGetPPC(cpu, inst_addr, node, code, pos, 1)) {
                                return 0;
                            }

                            (*inst_addr) -=4;
                        }

                        if(MIPS_RS(cur_inst) != MREG_RA) {
                            cpu->unk_0x12270 = 0;
                        }

                        if(MIPS_RS(cur_inst) == MREG_RA && !(cpu->unk_0x12220 & 2)) {
                            rA = reg_map[MIPS_RS(cur_inst)];
                            if(rA & 0x100) {
                                rA = 5;

                                if(code != NULL) {
                                    // lwz r5, cpu->gpr[rs]
                                    code[(*pos++)] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                                } else {
                                    (*pos)++;
                                }
                            }

                            if(code != NULL) {
                                // mtlr rA
                                code[(*pos)++] = 0x7C0803A6 | (rA << 21);
                                // blr
                                code[(*pos)++] = 0x4E800020;
                            }
                        } else {
                            rA = reg_map[MIPS_RS(cur_inst)];
                            if(rA & 0x100) {
                                if(code != NULL) {
                                    // lwz r5, cpu->gpr[rd]
                                    code[*pos] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                                }
                            } else {
                                if(code != NULL) {
                                    // mr r5, rA
                                    code[*pos] = 0x7C050378 | (rA << 21) | (rA << 11);
                                }
                            }
                            
                            (*pos)++;

                            if(code != NULL) {
                                offset = (u32)cpu->execute_jump - (u32)&code[*pos];
                                code[(*pos)++] = 0x48000001 | (offset & 0x3FFFFFC);
                            }
                        }

                        break;
                    case SPEC_JALR:
                        // Compile delay slot if not a NOP
                        if(next_inst != 0) {
                            if(!cpuGetPPC(cpu, inst_addr, node, code, pos, 1)) {
                                return 0;
                            }

                            (*inst_addr) -=4;
                        }

                        if(code != NULL) {
                            // lwz r5, cpu->running_node
                            code[*pos + 0] = 0x80A3002C;
                            // lis r7, 0x8000
                            code[*pos + 1] = 0x3ce08000;
                            // lwz r6, cpu->running_node->unk_0x28
                            code[*pos + 2] = 0x80c50028;
                            // or r6, r6, r7
                            code[*pos + 3] = 0x7cc63b78;
                            // lis r7, hi(cur_inst_addr + 8)
                            code[*pos + 4] = 0x3ce00000 | ((cur_inst_addr + 8) >> 0x10);
                            // stw r6, cpu->running_node->unk_0x28
                            code[*pos + 5] = 0x90c50028;
                            // ori r7, r7, lo(cur_inst_addr + 8)
                            code[*pos + 6] = 0x60e70000 | ((cur_inst_addr + 8) & 0xFFFF);
                        }

                        (*pos) += 7;

                        if(code != NULL) {
                            // stw r7, cpu->n64_ra
                            code[*pos] = 0x90E30030;
                        }

                        (*pos)++;

                        rB = reg_map[MREG_RA];
                        if(rB & 0x100) {
                            if(code != NULL) {
                                // lis r5, hi(ret_addr)
                                code[*pos] = 0x3CA00000 | ((u32)&code[*pos + 5] >> 0x10);
                                // ori r5, r5, lo(ret_addr)
                                code[*pos + 1] = 0x60A50000 | ((u32)&code[*pos + 5] & 0xFFFF);
                            }

                            (*pos) += 2;

                            if(code != NULL) {
                                (*pos)++;
                            } else {
                                // stw r5, cpu->gpr[MREG_RA]
                                code[(*pos)++] = 0x90A30000 + ((u32)&cpu->gpr[MREG_RA] - (u32)cpu + 4);
                            }
                        } else {
                            if(code != NULL) {
                                // lis rB, hi(ret_addr)
                                code[*pos] = 0x3C000000 | ((u32)&code[*pos + 5] >> 0x10) | (reg_map[MREG_RA] << 21);
                            }

                            (*pos)++;

                            if(code == NULL) {
                                (*pos)++;
                            }
                        }

                        rA = reg_map[MIPS_RS(cur_inst)];
                        if(rA & 0x100) {
                            if(code != NULL) {
                                // lwz r5, cpu->gpr[rs]
                                code[*pos] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                            }
                        } else {
                            if(code != NULL) {
                                // mr r5, rA
                                code[*pos] = 0x7C050378 | (rA << 21) | (rA << 11);
                            }
                        }

                        (*pos)++;

                        if(code != NULL) {
                            offset = (u32)cpu->execute_jump - (u32)&code[*pos];
                            // bl cpu->execute_jump
                            code[*pos + 0] = 0x48000001 | (offset & 0x3FFFFFC);
                            // lis r5, hi(node)
                            code[*pos + 1] = 0x3cA00000 | ((u32)node >> 0x10);
                            // ori r5, r5, lo(node)
                            code[*pos + 2] = 0x60A50000 | ((u32)node & 0xFFFF);
                            // stw r5, cpu->running_node
                            code[*pos + 3] = 0x90A3002C;
                            // lis r7, 0x8000
                            code[*pos + 4] = 0x3CE08000;
                            // lwz r6, node->unk_0x28
                            code[*pos + 5] = 0x80C50028;
                            // andc r6, r6, r7
                            code[*pos + 6] = 0x7CC63878;
                            // cmpwi r6, 0
                            code[*pos + 7] = 0x2C060000;
                            // beq + 8
                            code[*pos + 8] = 0x41820008;
                            // stw r4, node->unk_0x28
                            code[*pos + 9] = 0x90850028;
                        }

                        (*pos) += 10;

                        prev_pos = *pos;

                        if(!cpuGetPPC(cpu, inst_addr, node, code, pos, 0)) {
                            return 0;
                        }

                        if(code != NULL) {
                            offset = *pos - prev_pos;
                            // b from prev_pos to cur pos
                            code[prev_pos] = 0x48000000 | (offset & 0x3FFFFFC);
                        }

                        break;
                    case SPEC_MFHI:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RD(cur_inst)];
                        if(rA & 0x100) {
                            if(code != NULL) {
                                // lwz r5, lo32(cpu->hi)
                                code[*pos] = 0x80A30000 | ((u32)&cpu->hi - (u32)cpu + 4);
                                // stw r5, lo32(cpu->gpr[rd])
                                code[*pos + 1] = 0x90A30000 | ((u32)&cpu->gpr[rA] - (u32)cpu + 4);
                                // lwz r5, hi32(cpu->hi)
                                code[*pos + 2] = 0x80a30000 | ((u32)&cpu->hi - (u32)cpu);
                                // stw r5, hi32(cpu->gpr[rd])
                                code[*pos + 3] = 0x90a30000 | ((u32)&cpu->gpr[rA] - (u32)cpu);
                            }
                        } else {
                            if(code != NULL) {
                                // lwz rA, lo32(cpu->hi)
                                code[*pos] = 0x80030000 | (rA << 21) + ((u32)&cpu->hi - (u32)cpu + 4);
                                // lwz r5, hi32(cpu->hi)
                                code[*pos + 1] = 0x80a30000 | ((u32)&cpu->hi - (u32)cpu);
                                // stw r5, hi32(cpu->gpr[rd])
                                code[*pos + 2] = 0x90a30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu);
                            }
                        }

                        break;

                    case SPEC_MTHI:
                        rA = reg_map[MIPS_RS(cur_inst)];
                        if(rA & 0x100) {
                            if(code != NULL) {
                                // lwz r5, lo32(cpu->gpr[rs])
                                code[*pos] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                                // stw r5, lo32(cpu->hi)
                                code[*pos + 1] = 0x90A30000 + ((u32)&cpu->hi -(u32)cpu + 4);
                                // lwz r5, hi32(cpu->gpr[rs])
                                code[*pos + 2] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu);
                                // stw r5, hi32(cpu->hi)
                                code[*pos + 3] = 0x90A30000 + ((u32)&cpu->hi - (u32)cpu);
                            }
                        } else {
                            if(code != NULL) {
                                // stw rA, lo32(cpu->hi)
                                code[*pos] = 0x90030000 | (rA << 11) + ((u32)&cpu->hi - (u32)cpu + 4);
                                // lwz r5, hi32(cpu->gpr[rs])
                                code[*pos + 1] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu);
                                // stw r5, hi32(cpu->hi)
                                code[*pos + 2] = 0x90A30000 + ((u32)&cpu->hi - (u32)cpu);
                            }
                        }
                        break;
                    case SPEC_MFLO:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RD(cur_inst)];
                        if(rA & 0x100) {
                            if(code != NULL) {
                                // lwz r5, lo32(cpu->lo)
                                code[*pos] = 0x80A30000 | ((u32)&cpu->lo - (u32)cpu + 4);
                                // stw r5, lo32(cpu->gpr[rd])
                                code[*pos + 1] = 0x90A30000 | ((u32)&cpu->gpr[rA] - (u32)cpu + 4);
                                // lwz r5, hi32(cpu->lo)
                                code[*pos + 2] = 0x80a30000 | ((u32)&cpu->lo - (u32)cpu);
                                // stw r5, hi32(cpu->gpr[rd])
                                code[*pos + 3] = 0x90a30000 | ((u32)&cpu->gpr[rA] - (u32)cpu);
                            }
                        } else {
                            if(code != NULL) {
                                // lwz rA, lo32(cpu->lo)
                                code[*pos] = 0x80030000 | (rA << 21) + ((u32)&cpu->lo - (u32)cpu + 4);
                                // lwz r5, hi32(cpu->lo)
                                code[*pos + 1] = 0x80a30000 | ((u32)&cpu->lo - (u32)cpu);
                                // stw r5, hi32(cpu->gpr[rd])
                                code[*pos + 2] = 0x90a30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu);
                            }
                        }

                        break;
                    case SPEC_MTLO:
                        rA = reg_map[MIPS_RS(cur_inst)];
                        if(rA & 0x100) {
                            if(code != NULL) {
                                // lwz r5, lo32(cpu->gpr[rs])
                                code[*pos] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                                // stw r5, lo32(cpu->lo)
                                code[*pos + 1] = 0x90A30000 + ((u32)&cpu->lo -(u32)cpu + 4);
                                // lwz r5, hi32(cpu->gpr[rs])
                                code[*pos + 2] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu);
                                // stw r5, hi32(cpu->lo)
                                code[*pos + 3] = 0x90A30000 + ((u32)&cpu->lo - (u32)cpu);
                            }
                        } else {
                            if(code != NULL) {
                                // stw rA, lo32(cpu->lo)
                                code[*pos] = 0x90030000 | (rA << 11) + ((u32)&cpu->lo - (u32)cpu + 4);
                                // lwz r5, hi32(cpu->gpr[rs])
                                code[*pos + 1] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu);
                                // stw r5, hi32(cpu->lo)
                                code[*pos + 2] = 0x90A30000 + ((u32)&cpu->lo - (u32)cpu);
                            }
                        }
                        break;
                    case SPEC_DSLLV:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RT(cur_inst)];
                        if(!(rA & 0x100)) {
                            if(code != NULL) {
                                code[(*pos)++] = 0x90030000 | (rA << 11) + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);

                            } else {
                                (*pos)++;
                            }
                        }

                        rS = reg_map[MIPS_RS(cur_inst)];
                        if(!(rS & 0x100)) {
                            code[(*pos)++] = 0x90030000 | (rS << 11) + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                        } else {
                            (*pos)++;
                        }

                        if(code != NULL) {
                            // lwz r5, hi32(cpu->gpr[rt])
                            code[*pos + 0] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu);
                            // lwz r6, cpu->gpr[rt]
                            code[*pos + 1] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                            // lwz r7, cpu->gpr[rs]
                            code[*pos + 2] = 0x80E30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                            offset = (u32)cpuCompile_DSLLV_function - (u32)&code[*pos + 3];
                            // bl cpuCompile_DSLLV_function
                            code[*pos + 3] = 0x48000001 | (offset & 0x3FFFFFC);
                            // stw r5, hi32(cpu->gpr[rd])
                            code[*pos + 4] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] -(u32)cpu);
                        }

                        (*pos) += 5;

                        if(code != NULL) {
                            // stw, r6, cpu->gpr[rD]
                            code[*pos] = 0x90C30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                        }

                        (*pos)++;

                        rD = reg_map[MIPS_RD(cur_inst)];
                        if(!(rD & 0x100)) {
                            // mr rD, r6
                            code[*pos] = 0x7CC03378 | (rD << 16);
                        }
                        break;
                    case SPEC_DSRLV:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RT(cur_inst)];
                        if(!(rA & 0x100)) {
                            if(code != NULL) {
                                code[(*pos)++] = 0x90030000 | (rA << 11) + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);

                            } else {
                                (*pos)++;
                            }
                        }

                        rS = reg_map[MIPS_RS(cur_inst)];
                        if(!(rS & 0x100)) {
                            code[(*pos)++] = 0x90030000 | (rS << 11) + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                        } else {
                            (*pos)++;
                        }

                        if(code != NULL) {
                            // lwz r5, hi32(cpu->gpr[rt])
                            code[*pos + 0] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu);
                            // lwz r6, cpu->gpr[rt]
                            code[*pos + 1] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                            // lwz r7, cpu->gpr[rs]
                            code[*pos + 2] = 0x80E30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                            offset = (u32)cpuCompile_DSRLV_function - (u32)&code[*pos + 3];
                            // bl cpuCompile_DSLLV_function
                            code[*pos + 3] = 0x48000001 | (offset & 0x3FFFFFC);
                            // stw r5, hi32(cpu->gpr[rd])
                            code[*pos + 4] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] -(u32)cpu);
                        }

                        (*pos) += 5;

                        if(code != NULL) {
                            // stw, r6, cpu->gpr[rD]
                            code[*pos] = 0x90C30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                        }

                        (*pos)++;

                        rD = reg_map[MIPS_RD(cur_inst)];
                        if(!(rD & 0x100)) {
                            // mr rD, r6
                            code[*pos] = 0x7CC03378 | (rD << 16);
                        }
                        break;
                    case SPEC_DSRAV:
                        cpu->unk_0x1222C &= ~(1 << MIPS_RD(cur_inst));

                        rA = reg_map[MIPS_RT(cur_inst)];
                        if(!(rA & 0x100)) {
                            if(code != NULL) {
                                code[(*pos)++] = 0x90030000 | (rA << 11) + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);

                            } else {
                                (*pos)++;
                            }
                        }

                        rS = reg_map[MIPS_RS(cur_inst)];
                        if(!(rS & 0x100)) {
                            code[(*pos)++] = 0x90030000 | (rS << 11) + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                        } else {
                            (*pos)++;
                        }

                        if(code != NULL) {
                            // lwz r5, hi32(cpu->gpr[rt])
                            code[*pos + 0] = 0x80A30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu);
                            // lwz r6, cpu->gpr[rt]
                            code[*pos + 1] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(cur_inst)] - (u32)cpu + 4);
                            // lwz r7, cpu->gpr[rs]
                            code[*pos + 2] = 0x80E30000 + ((u32)&cpu->gpr[MIPS_RS(cur_inst)] - (u32)cpu + 4);
                            offset = (u32)cpuCompile_DSRAV_function - (u32)&code[*pos + 3];
                            // bl cpuCompile_DSLLV_function
                            code[*pos + 3] = 0x48000001 | (offset & 0x3FFFFFC);
                            // stw r5, hi32(cpu->gpr[rd])
                            code[*pos + 4] = 0x90A30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] -(u32)cpu);
                        }

                        (*pos) += 5;

                        if(code != NULL) {
                            // stw, r6, cpu->gpr[rD]
                            code[*pos] = 0x90C30000 + ((u32)&cpu->gpr[MIPS_RD(cur_inst)] - (u32)cpu + 4);
                        }

                        (*pos)++;

                        rD = reg_map[MIPS_RD(cur_inst)];
                        if(!(rD & 0x100)) {
                            // mr rD, r6
                            code[*pos] = 0x7CC03378 | (rD << 16);
                        }
                        break;
                }
                break;
        }
    }
}
#else
s32 cpuGetPPC(cpu_class_t *cpu, s32 *inst_addr, recomp_node_t *node, u32 *code, s32 *pos, s32 delay);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuGetPPC.s")
#endif

s32 func_80031D4C(cpu_class_t *cpu, recomp_node_t *node, s32 arg2) {
    return 0;
}

#ifdef NON_MATCHING
/**
 * @brief Creates a new recompiled function block.
 * 
 * @param cpu The emulated VR4300.
 * @param out_node A pointer to an already recompiled function, or one that has been created.
 * @param addr The n64 address of the function to find or create.
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuMakeFunction(cpu_class_t *cpu, recomp_node_t **out_node, s32 addr) {
    s32 success = 1;
    branch_t branches[0x400];
    s32 pos;
    s32 total_inst;
    s32 i;
    size_t code_size;
    size_t ext_call_size;
    size_t alloc_size;
    u32 *code;
    u32 *code_p;
    s32 ref_cnt;
    s32 cur_addr;
    recomp_node_t *node;

    if(!cpuFindFunction(cpu, addr, &node)) {
        return 0;
    }

    if(func_80031D4C(cpu, node, 1)) {
        if(out_node != NULL) {
            *out_node = node;
        }
        return 1;
    }

    if(node->recompiled_func == NULL) {
        libraryTestFunction(gSystem->unk_0x0058, node);
        node->branch_cnt = 0;
        node->branches = branches;
        cpu->unk_0x1221C = 0x20000000;
        cpu->unk_0x12220 = 0;
        node->unk_0x1C = 0;
        node->state = 11;
        cpu->unk_0x12270 = 1;
        cpu->unk_0x12294 = 0;
        pos = 0;
        cur_addr = node->n64_start_addr;
        while(cur_addr <= node->n64_end_addr) {
            if(!cpuGetPPC(cpu, &cur_addr, node, NULL, &pos, 0)) {
                return 0;
            }
        }
        total_inst = pos;
        code_size = pos * 4;
        alloc_size = code_size;
        if(node->ext_call_cnt != 0) {
            ext_call_size = node->ext_call_cnt * sizeof(*node->ext_calls);
            alloc_size += ext_call_size;
        } else {
            ext_call_size = 0;
        }

        if(node->branch_cnt > 0) {
            alloc_size += node->branch_cnt * sizeof(*node->branches);
        }

        do {
            s32 clean_cnt;
            recomp_tree_t *recomp_tree;
            if(cpuHeapTake((char**)&code, cpu, node, alloc_size)){
                break;
            }

            recomp_tree = cpu->recomp_tree;
            if(success) {
                success = 0;
                clean_cnt = cpu->call_cnt - 300;
                continue;
            }

            clean_cnt += 95;
            if(clean_cnt > (cpu->call_cnt - 10)) {
                clean_cnt = cpu->call_cnt - 10;
            }

            recomp_tree->unk_0x70 = 0;
            recomp_tree->unk_0x7C = NULL;
            recomp_tree->unk_0x80 = 0;
            if(node != NULL && node->unk_0x28 > 0) {
                node->unk_0x28 = cpu->call_cnt;
            }

            if(recomp_tree->unk_0x78 == 0) {
                if(recomp_tree->code_root != NULL) {
                    treeForceCleanNodes(cpu, recomp_tree->code_root, clean_cnt);
                }
            } else {
                if(recomp_tree->ovl_root != NULL) {
                    treeForceCleanNodes(cpu, recomp_tree->ovl_root, clean_cnt);
                }
            }

            recomp_tree->unk_0x78 ^= 1;
        } while(1);

        code_p = code;
        if(ext_call_size != 0) {
            ext_call_t *ext_call;
            node->ext_call = (ext_call_t*)((char*)code_p + code_size);
            ext_call = node->ext_call;
            for(i = 0; i < ref_cnt; i++) {
                ext_call[i].n64_addr = 0;
                ext_call[i].vc_addr = 0;
            }
        }

        cpu->unk_0x1221C = 0x20000000;
        cpu->unk_0x12220 = 0;
        node->unk_0x1C = 0;
        node->state = 22;
        cpu->unk_0x12294 = 0;
        cpu->unk_0x12274 = 0;
        cpu->unk_0x12280 = 0;
        pos = 0;
        cur_addr = node->n64_start_addr;
        while(cur_addr <= node->n64_end_addr) {
            if(!cpuGetPPC(cpu, &cur_addr, node, code_p, &pos, 0)) {
                return 0;
            }
        }

        if(pos > total_inst) {
            return 0;
        }

        while(pos < total_inst) {
            code_p[pos++] = 0x60000000; // nop;
        }

        node->state = 33;
        node->recompiled_func = (recomp_func_t*)code_p;
        DCStoreRange(node->recompiled_func, pos * 4);
        ICInvalidateRange(node->recompiled_func, pos * 4);
        if(node->branch_cnt > 0) {
            if(node->branch_cnt >= 0x400) {
                return 0;
            }

            node->branches = (branch_t*)((char*)code + code_size + ext_call_size);
            for(i = 0; i < node->branch_cnt; i++) {
                node->branches[i] = branches[i];
            }
        } else {
            node->branches = NULL;
        }

        node->size = alloc_size;
        cpu->recomp_tree->total_size += alloc_size;
        func_80031D4C(cpu, node, 0);
    }

    if(out_node != NULL) {
        *out_node = node;
    }

    return  1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuMakeFunction.s")
#endif

#ifdef NON_MATCHING
inline void setCache(cpu_class_t *cpu, u32 *code, recomp_node_t *node, s32 addr) {
    recomp_cache_t *cache;
    s32 cnt = cpu->cache_cnt;
    recomp_cache_t *cache_start = cpu->recomp_cache;

    if(cnt == 256) {
        cnt--;
    } else {
        cpu->cache_cnt++;
    }

    cache = &cache_start[cnt];

    while(cnt > 0) {
        *cache = cache[-1];
        cache--;
        cnt--;
    }

    cache_start->n64_addr = addr;
    cache_start->recomp_addr = code;
    cache_start->node = node;
}

/**
 * @brief Searches the recompiled block cache for an address, or creates a new block if one cannot be found.
 * 
 * @param cpu The emulated VR4300.
 * @param addr N64 code address to search for.
 * @param code A pointer to set the found PPC code to.
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuFindAddress(cpu_class_t *cpu, s32 addr, u32 **code) {
    s32 pos;
    s32 cur_addr;
    s32 i;
    s32 j;
    recomp_node_t *node;
    branch_t *branch;
    s32 cont;

    if(cpu->status & 0x20) {
        cpu->status &= ~0x20;
    }

    if(cpuFindCachedAddress(cpu, addr, code)) {
        return 1;
    }

    node = cpu->running_node;

    if(node == NULL || addr < node->n64_start_addr || node->n64_end_addr < addr) {
        if(!cpuMakeFunction(cpu, &node, addr)) {
            return 0;
        }
    }

    for(i = 0; i < node->branch_cnt; i++) {
        if(addr != node->branches[i].n64_target) {
            continue;
        }

        *code = &node->recompiled_func[node->branches[i].branch_dist];

        if(node->unk_0x28 > 0) {
            node->unk_0x28 = cpu->call_cnt;
        }

        setCache(cpu, *code, node, addr);
        return 1;
    }

    cpu->unk_0x1221C = 0x20000000;
    cpu->unk_0x12220 = 0;
    node->state = 33;
    pos = 0;
    if(node->n64_start_addr != addr) {
        node->unk_0x28 = 0;
    }

    cur_addr = node->n64_start_addr;
    while(cur_addr <= node->n64_end_addr) {
        if(cur_addr == addr) {
            *code = (u32*)((u32)node->recompiled_func + (pos * 4));
            if(node->unk_0x28 > 0) {
                node->unk_0x28 = cpu->call_cnt;
            }

            setCache(cpu, *code, node, addr);

            return 1;
        }

        if(!cpuGetPPC(cpu, &cur_addr, node, NULL, &pos, 0)) {
            return 0;
        }
    }

    return 0;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuFindAddress.s")
#endif
#undef NON_MATCHING

inline s32 find_in_branches(recomp_node_t *node, s32 pc) {
    s32 i;

    for(i = 0; i < node->branch_cnt; i++) {
        if(pc == node->branches[i].n64_target) {
            return 0;
        }
    }

    return 1;
}

s32 func_800326D0(cpu_class_t *cpu, s32 pc, s32 r5) {
    if(!cpu->unk_0x12270) {
        return 0;
    }

    if(cpu->unk_0x12274 == 0) {
        return 0;
    }

    if(r5 != cpu->tmp_mips_dest_reg) {
        return 0;
    }

    if(!find_in_branches(cpu->running_node, pc)) {
        cpu->unk_0x12274 = 0;
        return 0;
    }

    cpu->unk_0x12274 = 0;
    return 1;
}

s32 func_80032780(cpu_class_t *cpu, s32 pc, s32 r5) {
    if(!cpu->unk_0x12270) {
        return 0;
    }

    if(cpu->unk_0x12280 == 0) {
        return 0;
    }

    if(r5 != cpu->unk_0x12284) {
        return 0;
    }

    if(!find_in_branches(cpu->running_node, pc)) {
        cpu->unk_0x12280 = 0;
        return 0;
    }

    cpu->unk_0x12280 =0;
    return 1;
}

s32 func_80032830(cpu_class_t *cpu, s32 pc, u32 *code, s32 base, s32 rt) {
    s32 i;

    if(code == NULL) {
        return 0;
    }

    if(!cpu->unk_0x12270) {
        return 0;
    }

    if(!find_in_branches(cpu->running_node, pc)) {
        return 0;
    }

    cpu->unk_0x12288 = 1;

    if(base == rt) {
        cpu->unk_0x1228C = -1;
        return 0;
    }

    if(cpu->unk_0x1228C != base) {
        cpu->unk_0x1228C = base;
        return 0;
    }

    return 1;
}

s32 cpuNextInstruction(cpu_class_t *cpu, u32 addr, s32 inst, u32 *code, u32 *pos) {
    if(code == NULL) {
        return 0;
    }
   if(cpu->unk_0x12270 == 0) {
        return 0;
    }

    if(cpu->unk_0x12294 != addr - 4) {
        cpu->unk_0x12294 = 0;
        return 0;
    }
    
    cpu->unk_0x12294 = 0;

    if(!find_in_branches(cpu->running_node, addr)) {
        return 0;
    }

    switch(inst >> 0x1A) {
        case OPC_ORI:
            if(cpu->tmp_mips_dest_reg == MIPS_RS(inst) && MIPS_RS(inst) == MIPS_RT(inst)) {
                if(cpu->unk_0x12290 != 1000) {
                    return 0;
                }
                /**
                 * nop
                 * ori tmp_dest, tmp_dest, imm
                 * nop
                 */
                code[*pos - 1] = 0x60000000;
                code[(*pos)++] = (0x60000000 | (cpu->tmp_ppc_dest_reg << 0x15) | (cpu->tmp_ppc_dest_reg << 0x10)) | (inst & 0xFFFF);
                code[(*pos)++] = 0x60000000;
                code[(*pos)++] = ((cpu->tmp_ppc_dest_reg << 0x15) | 0x90030000) + ((u32)&cpu->gpr[MIPS_RT(inst)] - (u32)cpu + 4);
                cpu->unk_0x12274 = 2;
                return 1;
            }
            return 0;
        case OPC_ADDIU:
            if(cpu->tmp_mips_dest_reg == MIPS_RS(inst) && MIPS_RS(inst) == MIPS_RT(inst)) {
                if(cpu->unk_0x12290 != 1000) {
                    return 0;
                }

                /**
                 * nop
                 * addi tmp_dest, tmp_dest, imm
                 * nop
                 */
                code[*pos - 1] = 0x60000000;
                code[(*pos)++] = (inst & 0xFFFF) | (cpu->tmp_ppc_dest_reg << 0x10) | (cpu->tmp_ppc_dest_reg << 0x15) | 0x38000000;
                code[(*pos)++] = 0x60000000;
                code[(*pos)++] = ((cpu->tmp_ppc_dest_reg << 0x15) | 0x90030000) + ((u32)&cpu->gpr[MIPS_RT(inst)] - (u32)cpu + 4);
                cpu->unk_0x12274 = 2;
                return 1;
            }
            return 0;
        default:
            invalidInst();
            return 0;
    }

    return 0;
}

void func_80032B74(u32 arg0) {
    gSystem->cpu->unk_0x3C = arg0;
    if(lbl_8025CFE8 != NULL) {
        lbl_8025CFE8(arg0);
    }
}

#ifdef NON_MATCHING
inline s32 treeCheck(cpu_class_t *cpu, recomp_tree_t *tree) {
    s32 iVar1;

    if(cpu->call_cnt < 300) {
        return 0;
    }

    if(cpu->call_cnt == 300) {
        tree->unk_0x70 = 1;
        return 1;
    }

    
    if(cpu->call_cnt - cpu->call_cnt / 400 * 400 == 0) {
        if((cpu->recomp_tree != NULL ? cpu->recomp_tree->total_size : 0) > 0x319750) {
            tree->unk_0x70 = cpu->call_cnt - 200;
            return 1;
        }
    }

    return 0;
}

s32 cpuExecuteUpdate(cpu_class_t *cpu, u32 **code_p, u32 tick) {
    recomp_tree_t *tree;
    u32 cpu_tick;
    s32 val;

    if(!romUpdate(gSystem->rom)) {
        return 0;
    }


    if(!rspUpdate(gSystem->rsp, cpu->status & 0x80 && gSystem->unk_0x00 == 0 ? 1 : 0)) {
        return 0;
    }

    tree = cpu->recomp_tree;
    treeTimerCheck(cpu);
    if(cpu->unk_0x3C == cpu->unk_0x40 && tree->unk_0x74 < 12) {
        if(treeCheck(cpu, tree)) {
            cpu->unk_0x34++;
        }

        if(tree->unk_0x70 != 0) {
            treeCleanup(cpu, tree);
        }
    }

    cpu_tick = cpu->tick;

    if(tick > cpu_tick) {
        cpu_tick = (float)((tick - cpu_tick) * 4);
    } else {
        cpu_tick = (float)(((-1 - cpu_tick) + tick) * 4);
    }

    if(cpu->status & 0x40 && cpu->unk_0x40 != cpu->unk_0x3C && videoForceRetrace(gSystem->video)) {
        if(__abs(cpu->unk_0x3C - cpu->unk_0x40) < 4) {
            cpu->unk_0x40++;
        } else {
            cpu->unk_0x40 = cpu->unk_0x3C;
        }
    }


    if(cpu->status & 1) {
        if (((cpu->cp0[CP0_COMPARE].w[1] <= cpu->cp0[CP0_COUNT].w[1]) && (cpu->cp0[CP0_COUNT].w[1] + cpu_tick >= cpu->cp0[CP0_COMPARE].w[1])) ||
        ((cpu->cp0[CP0_COUNT].w[1] >= cpu->cp0[CP0_COMPARE].w[1] && ((cpu->cp0[CP0_COUNT].w[1] + cpu_tick >= cpu->cp0[CP0_COMPARE].w[1] && (cpu->cp0[CP0_COUNT].w[1] + cpu_tick < cpu->cp0[CP0_COUNT].w[1])))))) {
            cpu->status &= ~1;
            xlObjectEvent(gSystem, 0x1000, (void*)3);
        }

        cpu->cp0[CP0_COUNT].sd += cpu_tick;
    }

    if(cpu->status & 8 && !(cpu->status & 4) && gSystem->unk_0x00 != 0) {
        if(!systemCheckInterrupts(gSystem)) {
            return 0;
        }
    }

    if(cpu->status & 4) {
        cpu->status &= ~0x84;
        if(!cpuFindAddress(cpu, cpu->pc, code_p)) {
            return 0;
        }
    }

    return 1;
}
#else
s32 cpuExecuteUpdate(cpu_class_t *cpu, u32 **code_p, u32 tick);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteUpdate.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_DSLLV(cpu_class_t *cpu, u32 **out_func) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000040)) {
        return 0;
    }

    *out_func = buf;

    // stwu sp, -0x0018(sp)
    //andi. r7, r7, 0x1F
    // stw r9, 0x0010(sp)
    // subfic r9, r7, 32
    // stw r8, 0x0008(sp)
    // slw r5, r5, r7
    // srw r8, r6, r9
    // or r5, r5, r8
    // subi r9, r7, 32
    // slw r8, r6, r9
    // or r5, r5, r8
    // slw r6, r6, r7
    // lwz r8, 0x0008(sp)
    // lwz r9, 0x0010(sp)
    // addi sp, sp 24
    // blr

    buf[0] = 0x9421ffe8;
    buf[1] = 0x70e7003f;
    buf[2] = 0x91210010;
    buf[3] = 0x21270020;
    buf[4] = 0x91010008;
    buf[5] = 0x7ca53830;
    buf[6] = 0x7cc84c30;
    buf[7] = 0x7ca54378;
    buf[8] = 0x3927ffe0;
    buf[9] = 0x7cc84830;
    buf[10] = 0x7ca54378;
    buf[11] = 0x7cc63830;
    buf[12] = 0x81010008;
    buf[13] = 0x81210010;
    buf[14] = 0x38210018;
    buf[15] = 0x4E800020;

    DCStoreRange(buf, 0x40);
    ICInvalidateRange(buf, 0x40);
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DSLLV.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_DSRLV(cpu_class_t *cpu, u32 **code) {
    u32 *buf;
    u32 pos;

    if(!xlHeapTake((void**)&buf, 0x30000040)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x9421ffe8;
    buf[1] = 0x70e7003f;
    buf[2] = 0x91210010;
    buf[3] = 0x21270020;
    buf[4] = 0x91010008;
    buf[5] = 0x7cc63c30;
    buf[6] = 0x7ca84830;
    buf[7] = 0x7cc64378;
    buf[8] = 0x3927ffe0;
    buf[9] = 0x7ca84c30;
    buf[10] = 0x7cc64378;
    buf[11] = 0x7ca53c30;
    buf[12] = 0x81010008;
    buf[13] = 0x81210010;
    buf[14] = 0x38210018;
    buf[15] = 0x4E800020;

    DCStoreRange(buf, 0x40);
    ICInvalidateRange(buf, 0x40);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DSRLV.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_DSRAV(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000044)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x9421ffe8;
    buf[1] = 0x70e7003f;
    buf[2] = 0x91210010;
    buf[3] = 0x21270020;
    buf[4] = 0x91010008;
    buf[5] = 0x7cc63c30;
    buf[6] = 0x7ca84830;
    buf[7] = 0x7cc64378;
    buf[8] = 0x3527ffe0;
    buf[9] = 0x7ca84e30;
    buf[10] = 0x40810008;
    buf[11] = 0x61060000;
    buf[12] = 0x7ca53e30;
    buf[13] = 0x81010008;
    buf[14] = 0x81210010;
    buf[15] = 0x38210018;
    buf[16] = 0x4E800020;

    DCStoreRange(buf, 0x44);
    ICInvalidateRange(buf, 0x44);
    
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DSRAV.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_DMULT(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x300000D4)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x39200000;
    buf[1] = 0x39400000;
    buf[2] = 0x39800040;
    buf[3] = 0x39600001;
    buf[4] = 0x2c050000;
    buf[5] = 0x40800014;
    buf[6] = 0x7cc630f8;
    buf[7] = 0x7ca528f8;
    buf[8] = 0x7cc65814;
    buf[9] = 0x7ca54914;
    buf[10] = 0x2c070000;
    buf[11] = 0x40800014;
    buf[12] = 0x7d0840f8;
    buf[13] = 0x7ce738f8;
    buf[14] = 0x7d085814;
    buf[15] = 0x7ce74914;
    buf[16] = 0x710b0001;
    buf[17] = 0x41820018;
    buf[18] = 0x39600000;
    buf[19] = 0x7d4a3014;
    buf[20] = 0x7d292914;
    buf[21] = 0x7d6b5914;
    buf[22] = 0x42800008;
    buf[23] = 0x39600000;
    buf[24] = 0x5508f87e;
    buf[25] = 0x50e8f800;
    buf[26] = 0x54e7f87e;
    buf[27] = 0x5147f800;
    buf[28] = 0x554af87e;
    buf[29] = 0x512af800;
    buf[30] = 0x5529f87e;
    buf[31] = 0x5169f800;
    buf[32] = 0x556bf87e;
    buf[33] = 0x398cffff;
    buf[34] = 0x2c0c0000;
    buf[35] = 0x4082ffb4;
    buf[36] = 0x39600001;
    buf[37] = 0x7dce7a78;
    buf[38] = 0x2c0e0000;
    buf[39] = 0x40800024;
    buf[40] = 0x7d0840f8;
    buf[41] = 0x7ce738f8;
    buf[42] = 0x7d4a50f8;
    buf[43] = 0x7d2948f8;
    buf[44] = 0x7d085814;
    buf[45] = 0x7ce76114;
    buf[46] = 0x7d4a6114;
    buf[47] = 0x7d296114;
    buf[48] = 0x9103000c;
    buf[49] = 0x90e30008;
    buf[50] = 0x91430014;
    buf[51] = 0x91230010;
    buf[52] = 0x4E800020;

    DCStoreRange(buf, 0xD4);
    ICInvalidateRange(buf, 0xD4);

    return 1;

}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DMULT.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_DMULTU(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000070)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x39200000;
    buf[1] = 0x39400000;
    buf[2] = 0x39800040;
    buf[3] = 0x710b0001;
    buf[4] = 0x41820018;
    buf[5] = 0x39600000;
    buf[6] = 0x7d4a3014;
    buf[7] = 0x7d292914;
    buf[8] = 0x7d6b5914;
    buf[9] = 0x42800008;
    buf[10] = 0x39600000;
    buf[11] = 0x5508f87e;
    buf[12] = 0x50e8f800;
    buf[13] = 0x54e7f87e;
    buf[14] = 0x5147f800;
    buf[15] = 0x554af87e;
    buf[16] = 0x512af800;
    buf[17] = 0x5529f87e;
    buf[18] = 0x5169f800;
    buf[19] = 0x556bf87e;
    buf[20] = 0x398cffff;
    buf[21] = 0x2c0c0000;
    buf[22] = 0x4082ffb4;
    buf[23] = 0x91030000 + ((u32)&cpu->lo - (u32)cpu + 4);
    buf[24] = 0x90e30000 + ((u32)&cpu->lo - (u32)cpu);
    buf[25] = 0x91430000 + ((u32)&cpu->hi - (u32)cpu + 4);
    buf[26] = 0x91230000 + ((u32)&cpu->hi - (u32)cpu);
    buf[27] = 0x4e800020;

    DCStoreRange(buf, 0x70);
    ICInvalidateRange(buf, 0x70);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DMULTU.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_DDIV(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000100)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x38a00040;
    buf[1] = 0x38c00000;
    buf[2] = 0x38e00000;
    buf[3] = 0x39800001;
    buf[4] = 0x2c080000;
    buf[5] = 0x40800014;
    buf[6] = 0x7d2948f8;
    buf[7] = 0x7d0840f8;
    buf[8] = 0x7d296014;
    buf[9] = 0x7d083114;
    buf[10] = 0x2c0a0000;
    buf[11] = 0x40800014;
    buf[12] = 0x7d6b58f8;
    buf[13] = 0x7d4a50f8;
    buf[14] = 0x7d6b6014;
    buf[15] = 0x7d4a3114;
    buf[16] = 0x3d80ffff;
    buf[17] = 0x618cfffe;
    buf[18] = 0x2c060000;
    buf[19] = 0x4180002c;
    buf[20] = 0x54c6083c;
    buf[21] = 0x50e60ffe;
    buf[22] = 0x54e7083c;
    buf[23] = 0x51070ffe;
    buf[24] = 0x5508083c;
    buf[25] = 0x51280ffe;
    buf[26] = 0x5529083c;
    buf[27] = 0x7ceb3810;
    buf[28] = 0x7cca3110;
    buf[29] = 0x42800028;
    buf[30] = 0x54c6083c;
    buf[31] = 0x50e60ffe;
    buf[32] = 0x54e7083c;
    buf[33] = 0x51070ffe;
    buf[34] = 0x5508083c;
    buf[35] = 0x51280ffe;
    buf[36] = 0x5529083c;
    buf[37] = 0x7ce75814;
    buf[38] = 0x7cc65114;
    buf[39] = 0x2c060000;
    buf[40] = 0x4180000c;
    buf[41] = 0x61290001;
    buf[42] = 0x42800008;
    buf[43] = 0x7d296038;
    buf[44] = 0x38a5ffff;
    buf[45] = 0x2c050000;
    buf[46] = 0x4082ff90;
    buf[47] = 0x2c060000;
    buf[48] = 0x4080000c;
    buf[49] = 0x7ce75814;
    buf[50] = 0x7cc65114;
    buf[51] = 0x39800001;
    buf[52] = 0x7dce7a78;
    buf[53] = 0x2c0e0000;
    buf[54] = 0x40800014;
    buf[55] = 0x7d2948f8;
    buf[56] = 0x7d0840f8;
    buf[57] = 0x7d296014;
    buf[58] = 0x7d082914;
    buf[59] = 0x91030000 + ((u32)&cpu->lo - (u32)cpu);
    buf[60] = 0x91230000 + ((u32)&cpu->lo - (u32)cpu + 4);
    buf[61] = 0x90c30000 + ((u32)&cpu->hi - (u32)cpu);
    buf[62] = 0x90e30000 + ((u32)&cpu->hi - (u32)cpu + 4);
    buf[63] = 0x4E800020;

    DCStoreRange(buf, 0x100);
    ICInvalidateRange(buf, 0x100);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DDIV.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_DDIVU(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x300000AC)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x38a00040;
    buf[1] = 0x38c00000;
    buf[2] = 0x38e00000;
    buf[3] = 0x3d80ffff;
    buf[4] = 0x618cfffe;
    buf[5] = 0x2c060000;
    buf[6] = 0x4180002c;
    buf[7] = 0x54c6083c;
    buf[8] = 0x50e60ffe;
    buf[9] = 0x54e7083c;
    buf[10] = 0x51070ffe;
    buf[11] = 0x5508083c;
    buf[12] = 0x51280ffe;
    buf[13] = 0x5529083c;
    buf[14] = 0x7ceb3810;
    buf[15] = 0x7cca3110;
    buf[16] = 0x42800028;
    buf[17] = 0x54c6083c;
    buf[18] = 0x50e60ffe;
    buf[19] = 0x54e7083c;
    buf[20] = 0x51070ffe;
    buf[21] = 0x5508083c;
    buf[22] = 0x51280ffe;
    buf[23] = 0x5529083c;
    buf[24] = 0x7ce75814;
    buf[25] = 0x7cc65114;
    buf[26] = 0x2c060000;
    buf[27] = 0x4180000c;
    buf[28] = 0x61290001;
    buf[29] = 0x42800008;
    buf[30] = 0x7d296038;
    buf[31] = 0x38a5ffff;
    buf[32] = 0x2c050000;
    buf[33] = 0x4082ff90;
    buf[34] = 0x2c060000;
    buf[35] = 0x4080000c;
    buf[36] = 0x7ce75814;
    buf[37] = 0x7cc65114;
    buf[38] = 0x91030000 + ((u32)&cpu->lo - (u32)cpu);
    buf[39] = 0x91230000 + ((u32)&cpu->lo - (u32)cpu + 4);
    buf[40] = 0x90c30000 + ((u32)&cpu->hi - (u32)cpu);
    buf[41] = 0x90e30000 + ((u32)&cpu->hi - (u32)cpu + 4);
    buf[42] = 0x4E800020;

    DCStoreRange(buf, 0xAC);
    ICInvalidateRange(buf, 0xAC);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DDIVU.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_S_SQRT(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000090)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0xc0030000 + ((u32)cpu->gpr - (u32)cpu + 4);
    buf[1] = 0xfc010040;
    buf[2] = 0x40810078;
    buf[3] = 0xfc400834;
    buf[4] = 0x3ca03fe0;
    buf[5] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu);
    buf[6] = 0xc8830000 + ((u32)cpu->gpr - (u32)cpu);
    buf[7] = 0x3ca04008;
    buf[8] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu);
    buf[9] = 0xc8630000 + ((u32)cpu->gpr - (u32)cpu);
    buf[10] = 0xfca400b2;
    buf[11] = 0xfcc200b2;
    buf[12] = 0xfcc101b2;
    buf[13] = 0xfcc33028;
    buf[14] = 0xfc4501b2;
    buf[15] = 0xfca400b2;
    buf[16] = 0xfcc200b2;
    buf[17] = 0xfcc101b2;
    buf[18] = 0xfcc33028;
    buf[19] = 0xfc4501b2;
    buf[20] = 0xfca400b2;
    buf[21] = 0xfcc200b2;
    buf[22] = 0xfcc101b2;
    buf[23] = 0xfcc33028;
    buf[24] = 0xfc4501b2;
    buf[25] = 0xfca400b2;
    buf[26] = 0xfcc200b2;
    buf[27] = 0xfcc101b2;
    buf[28] = 0xfcc33028;
    buf[29] = 0xfc4501b2;
    buf[30] = 0xfcc100b2;
    buf[31] = 0xfc203018;
    buf[32] = 0x38a00000;
    buf[33] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu + 4);
    buf[34] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu);
    buf[35] = 0x4e800020;

    DCStoreRange(buf, 0x90);
    ICInvalidateRange(buf, 0x90);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_S_SQRT.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_D_SQRT(cpu_class_t *cpu, u32 **code) {
    u32 *buf;
    
    if(!xlHeapTake((void**)&buf, 0x300000C0)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0xc80300 + ((u32)cpu->gpr - (u32)cpu);
    buf[1] = 0xfc010040;
    buf[2] = 0x40810068;
    buf[3] = 0xfc400834;
    buf[4] = 0x3ca03fe0;
    buf[5] = 0x90a300 + ((u32)cpu->gpr - (u32)cpu);
    buf[6] = 0xc88300 + ((u32)cpu->gpr - (u32)cpu);
    buf[7] = 0x3ca04008;
    buf[8] = 0x90a300 + ((u32)cpu->gpr - (u32)cpu);
    buf[9] = 0xc86300 + ((u32)cpu->gpr - (u32)cpu);
    buf[10] = 0xfc0200b2;
    buf[0xb] = 0xfc4400b2;
    buf[0xc] = 0xfc01183c;
    buf[0xd] = 0xfc420032;
    buf[0xe] = 0xfc0200b2;
    buf[0xf] = 0xfc4400b2;
    buf[0x10] = 0xfc01183c;
    buf[0x11] = 0xfc420032;
    buf[0x12] = 0xfc0200b2;
    buf[0x13] = 0xfc4400b2;
    buf[0x14] = 0xfc01183c;
    buf[0x15] = 0xfc420032;
    buf[0x16] = 0xfc0200b2;
    buf[0x17] = 0xfc4400b2;
    buf[0x18] = 0xfc01183c;
    buf[0x19] = 0xfc020032;
    buf[0x1a] = 0xfc210032;
    buf[0x1b] = 0x42800044;
    buf[0x1c] = 0xfc010000;
    buf[0x1d] = 0x4082000c;
    buf[0x1e] = 0xfc200090;
    buf[0x1f] = 0x42800034;
    buf[0x20] = 0xfc010000;
    buf[0x21] = 0x41820020;
    buf[0x22] = 0x3ca07fff;
    buf[0x23] = 0x60a5ffff;
    buf[0x24] = 0x90a300 + ((u32)cpu->gpr - (u32)cpu);
    buf[0x25] = 0x3ca0e000;
    buf[0x26] = 0x90a3004c;
    buf[0x27] = 0xc02300 + ((u32)cpu->gpr - (u32)cpu);
    buf[0x28] = 0x42800010;
    buf[0x29] = 0x3ca07ff0;
    buf[0x2a] = 0x90a300 + ((u32)cpu->gpr - (u32)cpu);
    buf[0x2b] = 0xc02300 + ((u32)cpu->gpr - (u32)cpu);
    buf[0x2c] = 0x38a00000;
    buf[0x2d] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu + 4);
    buf[0x2e] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu);
    buf[0x2f] = 0x48E00020;

    DCStoreRange(buf, 0xC0);
    ICInvalidateRange(buf, 0xC0);

    return 1;

}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_D_SQRT.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_W_CVT_SD(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000038)) {
        return 0;
    }

    *code = buf;

    /**
     * xoris r5, r5, 0x8000
     * lis r6, 0x4330
     * stw r5, cpu->gpr[0].w[1]
     * stw r6, cpu->gpr[0].w[0]
     * lfd f0, cpu->gpr[0].fd
     * stw r6, cpu->gpr[0].w[0]
     * lis r5, 0x8000
     * stw r5, cpu->gpr[0].w[1]
     * lfd f1, cpu->gpr[0].fd
     * fsub f1, f0, f1
     * li r5, 0
     * stw r5, cpu->gpr[0].w[0]
     * stw r5, cpu->gpr[0].w[1]
     * blr
     */

    buf[0] = 0x6ca58000; 
    buf[1] = 0x3cc04330;
    buf[2] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu + 4);
    buf[3] = 0x90c30000 + ((u32)cpu->gpr - (u32)cpu);
    buf[4] = 0xc8030000 + ((u32)cpu->gpr - (u32)cpu);
    buf[5] = 0x90c30000 + ((u32)cpu->gpr - (u32)cpu);
    buf[6] = 0x3ca08000;
    buf[7] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu + 4);
    buf[8] = 0xc8230000 + ((u32)cpu->gpr - (u32)cpu);
    buf[9] = 0xfc200828;
    buf[10] = 0x38a00000;
    buf[11] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu);
    buf[12] = 0x90a30000 + ((u32)cpu->gpr - (u32)cpu + 4);
    buf[13] = 0x4E800020;

    DCStoreRange(buf, 0x38);
    ICInvalidateRange(buf, 0x38);

    return 1;

}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_W_CVT_SD.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_L_CVT_SD(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x300000E0)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x9421ffd0;
    buf[1] = 0x91010008;
    buf[2] = 0x91210010;
    buf[3] = 0x91410018;
    buf[4] = 0x91610020;
    buf[5] = 0x91810028;
    buf[6] = 0x9421fff0;
    buf[7] = 0x54a70001;
    buf[8] = 0x4182000c;
    buf[9] = 0x20c60000;
    buf[10] = 0x7ca50190;
    buf[0xb] = 0x7ca93379;
    buf[0xc] = 0x39000000;
    buf[0xd] = 0x41820080;
    buf[0xe] = 0x7ca90034;
    buf[0xf] = 0x7cca0034;
    buf[0x10] = 0x552bd008;
    buf[0x11] = 0x7d6bfe70;
    buf[0x12] = 0x7d6b5038;
    buf[0x13] = 0x7d295a14;
    buf[0x14] = 0x21490020;
    buf[0x15] = 0x3169ffe0;
    buf[0x16] = 0x7ca54830;
    buf[0x17] = 0x7ccc5430;
    buf[0x18] = 0x7ca56378;
    buf[0x19] = 0x7ccc5830;
    buf[0x1a] = 0x7ca56378;
    buf[0x1b] = 0x7cc64830;
    buf[0x1c] = 0x7d094050;
    buf[0x1d] = 0x54c9057e;
    buf[0x1e] = 0x2c090400;
    buf[0x1f] = 0x3908043e;
    buf[0x20] = 0x4180001c;
    buf[0x21] = 0x4181000c;
    buf[0x22] = 0x54c90529;
    buf[0x23] = 0x41820010;
    buf[0x24] = 0x30c60800;
    buf[0x25] = 0x7ca50194;
    buf[0x26] = 0x7d080194;
    buf[0x27] = 0x54c6a83e;
    buf[0x28] = 0x50a6a814;
    buf[0x29] = 0x54a5ab3e;
    buf[0x2a] = 0x5508a016;
    buf[0x2b] = 0x7d052b78;
    buf[0x2c] = 0x7ce52b78;
    buf[0x2d] = 0x90a10008;
    buf[0x2e] = 0x90c1000c;
    buf[0x2f] = 0xc8210008;
    buf[0x30] = 0x38210010;
    buf[0x31] = 0x81010008;
    buf[0x32] = 0x81210010;
    buf[0x33] = 0x81410018;
    buf[0x34] = 0x81610020;
    buf[0x35] = 0x81810028;
    buf[0x36] = 0x38210030;
    buf[0x37] = 0x48E00020;

    DCStoreRange(buf, 0xE0);
    ICInvalidateRange(buf, 0xE0);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_L_CVT_SD.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_CEIL_W(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000034)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0x9421ffe0;
    buf[1] = 0xc8030000 + ((u32)cpu->fpr - (u32)cpu) & 0xFFFF;
    buf[2] = 0xfc010040;
    buf[3] = 0x4081000c;
    buf[4] = 0x38c00001;
    buf[5] = 0x42800008;
    buf[6] = 0x38c00000;
    buf[7] = 0xfc20081e;
    buf[8] = 0xd8210010;
    buf[9] = 0x80a10014;
    buf[10] = 0x7ca62a14;
    buf[0xb] = 0x38210020;
    buf[0xc] = 0x4E800020;

    DCStoreRange(buf, 0x34);
    ICInvalidateRange(buf, 0x34);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_CEIL_W.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_FLOOR_W(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000034)) {
        return 0;
    }

    *code = buf;

    /**
     * stwu sp, -32(sp)
     * lfd f0, cpu->fpr[0]
     * fcmpo cr0, f1, f0
     * blt- -> buf[6]
     * li r6, 0
     * bc 20, 0 -> buf[7]
     * li r6, 1
     * fctiwz f1, f1
     * stfd f1, 0x0010(sp)
     * lwz r5, 0x0014(sp)
     * sub r5, r5, r6
     * addi sp, sp, 32
     * blr
     */

    buf[0] = 0x9421ffe0;
    buf[1] = 0xc8030000 + (((u32)&cpu->fpr[0] - (u32)cpu) & 0xFFFF);
    buf[2] = 0xfc010040;
    buf[3] = 0x4180000c;
    buf[4] = 0x38c00000;
    buf[5] = 0x42800008;
    buf[6] = 0x38c00001;
    buf[7] = 0xfc20081e;
    buf[8] = 0xd8210010;
    buf[9] = 0x80a10014;
    buf[10] = 0x7ca62850;
    buf[11] = 0x38210020;
    buf[12] = 0x4E800020;

    DCStoreRange(buf, 0x34);
    ICInvalidateRange(buf, 0x34);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_FLOOR_W.s")
#endif

#ifdef NON_MATCHING

inline s32 rlwinm(u32 *code, u32 ra, u32 rs, u32 sh, u32 mb, u32 me) {
    *code = (21 << 26) | (rs << 21) | (ra << 16) | (sh << 11) | (mb << 6) | (me << 1);

}

inline s32 addi(u32 *code, u32 rt, u32 ra, u32 imm) {
    *code = ((14 << 26) | (rt << 21) | (ra << 16)) + imm;
}

inline s32 lwzx(u32 *code, u32 rt, u32 ra, u32 rb) {
    *code = (31 << 26) | (rt << 21) | (ra << 16) | (rb << 11) | (23 << 1);
}

inline s32 lwz(u32 *code, u32 rt, u32 ra, u32 d) {
    *code = ((32 << 26) | (rt << 21) | (ra << 16)) + d;
}

inline s32 add(u32 *code, u32 rt, u32 ra, u32 rb) {
    *code= (31 << 26) | (rt << 21) | (ra << 16) | (rb << 11) | (266 << 1);
}

inline s32 lbzx(u32 *code, u32 rt, u32 ra, u32 rb) {
    *code = (31 << 26) | (rt << 21) | (ra << 16) | (rb << 11) | (87 << 1);
}

inline s32 extsb(u32 *code,  u32 ra, u32 rs) {
    *code = (31 << 26) | (rs << 21) | (ra << 16) | (954 << 1);
}

inline void blr(u32 *code) {
    *code = (19 << 26) | (20 << 21) | (16 << 1);
}

s32 cpuCompile_LB(cpu_class_t *cpu, void **code) {
    u32 *buf;
    s32 pos;

    if(!xlHeapTake((void**)&buf, 0x3000002C)) {
        return 0;
    }

    *code = buf;

    // r3 = cpu
    // r5 = address
    // r6 = dev_idx
    // rlwinm r6, r6, 2, 0, 29; r6 = (dev_idx * 4) & 0xC
    // addi r7, r3 (offsetOf devices); r7 = cpu->devices;
    // lwzx r6, r6, r7; r6 = cpu->devices[dev_idx]
    // lwz r7, 0x0008(r6); r7 = cpu->devices[dev_idx].addr_offset
    // add r5, r5, r7; r5 = address + cpu->devices[dev_idx].addr_offset
    // lwz r7, 0x0004(r6); r7 = cpu->devices[dev_idx].dev_obj
    // lwz r7, 0x0004(r7); r7 cpu->devices[dev_idx].dev_obj + 4
    // lbzx r5, r5, r7
    // extsb r5, r5
    // blr

    pos = 0;

    rlwinm(&buf[pos++], 6, 6, 2, 0, 29);
    addi(&buf[pos++], 7, 3, ((s32)&cpu->devices[0] - (s32)cpu));
    lwzx(&buf[pos++], 6, 6, 7);
    lwz(&buf[pos++], 7, 6, offsetof(cpu_dev_t, addr_offset));
    add(&buf[pos++], 5, 5, 7);
    lwz(&buf[pos++], 7, 6, offsetof(cpu_dev_t, dev_obj));
    lwz(&buf[pos++], 7, 7, offsetof(ram_class_t, dram));
    lbzx(&buf[pos++], 5, 5, 7);
    extsb(&buf[pos++], 5, 5);
    blr(&buf[pos++]);

    DCStoreRange(buf, pos * sizeof(*buf));
    ICInvalidateRange(buf, pos * sizeof(*buf));

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LB.s")
#endif
#undef NON_MATCHING

#ifdef NON_MATCHING
s32 cpuCompile_LH(cpu_class_t *cpu, u32 **code) {
    u32 *buf;
    if(!xlHeapTake((void**)&buf, 0x3000002C)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0x54c6103a;
    buf[1] = 0x38e30000 + ((u32)cpu->devices - (u32)cpu);
    buf[2] = 0x7cc6382e;
    buf[3] = 0x80e60008;
    buf[4] = 0x7ca53a14;
    buf[5] = 0x80e60004;
    buf[6] = 0x80e70004;
    buf[7] = 0x7ca53a2e;
    buf[8] = 0x7ca50734;
    buf[9] = 0x4E800020;

    DCStoreRange(buf, 0x28);
    ICInvalidateRange(buf, 0x28);
    
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LH.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_LW(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000028)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x54c6103a; 
    buf[1] = 0x38e30000 + ((u32)cpu->devices - (u32)cpu);
    buf[2] = 0x7cc6382e;
    buf[3] = 0x80e60000 + offsetof(cpu_dev_t, addr_offset);
    buf[4] = 0x7ca53a14;
    buf[5] = 0x80e60000 + offsetof(cpu_dev_t, dev_obj);
    buf[6] = 0x80e70000 + offsetof(ram_class_t, dram);
    buf[7] = 0x7ca5382e;
    buf[8] = 0x4E800020;

    DCStoreRange(buf, 0x24);
    ICInvalidateRange(buf, 0x24);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LW.s")
#endif

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LBU.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LHU.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_SB.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_SH.s")

#ifdef NON_MATCHING
s32 cpuCompile_SW(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000028)) {
        return 0;
    }

    *code = buf;

    // rlwinm r6, r6, 2, 0, 29
    // addi r7, r3, cpu->devices
    // lwzx r6, r6, r7
    // lwz r7, r7, 0x0008(r6)
    // add r5, r5, r7
    // lwz r7, 0x0004(r6)
    // lwz r7, 0x0004(r7)
    // stwx r8, r5, r7
    // blr

    buf[0] = 0x54c6103a;
    buf[1] = 0x38e30000 + ((u32)cpu->devices - (u32)cpu);
    buf[2] = 0x7cc6382e;
    buf[3] = 0x80e60008;
    buf[4] = 0x7ca53a14;
    buf[5] = 0x80e60004;
    buf[6] = 0x80e70004;
    buf[7] = 0x7d05392e;
    buf[8] = 0x4E800020;

    DCStoreRange(buf, 0x24);
    ICInvalidateRange(buf, 0x24);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_SW.s")
#endif

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LDC.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_SDC.s")

#ifdef NON_MATCHING
s32 cpuCompile_LWL(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000030)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x38c00018;
    buf[1] = 0x88a70000;
    buf[2] = 0x7ca53030;
    buf[3] = 0x394000ff;
    buf[4] = 0x7d4a3030;
    buf[5] = 0x7d295078;
    buf[6] = 0x7d292b78;
    buf[7] = 0x38c6fff8;
    buf[8] = 0x54e507bf;
    buf[9] = 0x38e70001;
    buf[10] = 0x4082ffdc;
    buf[11] = 0x4e800020;

    DCStoreRange(buf, 0x30);
    ICInvalidateRange(buf, 0x30);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LWL.s")
#endif

#ifdef NON_MATCHING
s32 cpuCompile_LWR(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000030)) {
        return 0;
    }

    *code = buf;

    buf[0] = 0x38c00000;
    buf[1] = 0x88a70000;
    buf[2] = 0x7ca53030;
    buf[3] = 0x394000ff;
    buf[4] = 0x7d4a3030;
    buf[5] = 0x7d295078;
    buf[6] = 0x7d292b78;
    buf[7] = 0x38c60008;
    buf[8] = 0x54e507bf;
    buf[9] = 0x38e7ffff;
    buf[10] = 0x4082ffdc;
    buf[11] = 0x4e800020;

    DCStoreRange(buf, 0x30);
    ICInvalidateRange(buf, 0x30);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LWR.s")
#endif

#ifdef NON_MATCHING
inline s32 checkRetrace() {
    if(gSystem->unk_0x00 != 0) {
        if(!systemCheckInterrupts(gSystem)) {
            return 0;
        }
    } else {    
        videoForceRetrace(gSystem->video);
    }

    return 1;
}

inline unkFrameSet(u32 rom_id, u32 *frame, u32 addr) {
    if(rom_id == 'NKTJ') {
        if(addr == 0x802A4118) {
            frame[0x11] = 0;
        }
        if(addr == 0x800729D4) {
            frame[0x11] = 1;
        }
    } else if(rom_id == 'NKTP') {
        if(addr == 0x802A4160) {
            frame[0x11] = 0;
        }
        if(addr == 0x80072E34) {
            frame[0x11] = 1;
        }
    } else if(rom_id == 'NKTE') {
        if(addr == 0x802A4160) {
            frame[0x11] = 0;
        }
        if(addr == 0x80072E54) {
            frame[0x11] = 1;
        }
    }
}

inline cpu_dev_t *get_dev(cpu_dev_t **devs, u8 *idxs, u32 addr) {
    return devs[idxs[(addr >> 0x10)]];
}

inline void setCP0Random(cpu_class_t *cpu) {
    s32 val;
    s32 i;

    for(val = 0, i = 0; i < 48; i++) {
        if(!(cpu->tlb[i].entry_hi.sw[1] & 2)) {
            val++;
        }
    }

    cpu->cp0[CP0_RANDOM].sd = val;

    cpuSetTLB(cpu, cpu->cp0[CP0_RANDOM].sw[1]);
}

void *cpuExecuteOpcode(cpu_class_t *cpu, s32 arg1, u32 addr, u32 *ret) {
    u64 prev_ra;
    s32 save_ra = 0;
    u32 tick = OSGetTick();
    u32 inst;
    cpu_dev_t **devs;
    cpu_dev_t *dev;
    s32 i;
    s32 val;
    s32 sh;
    u32 ea;
    u8 *dev_idx;
    u32 *code_buf;
    u8 byteBuf;
    u16 halfBuf;
    u32 wordBuf;
    reg64_t tmp;

    if(cpu->unk_0x24 != 0) {
        cpu->status |= 8;
    } else {
        cpu->status &= ~8;
    }

    unkFrameSet(gSystem->rom_id, gSystem->frame, addr);

    dev_idx = cpu->mem_hi_map;
    devs = cpu->devices;

    if(!cpuGetAddressBuffer(cpu, (void**)&code_buf, addr)) {
        return NULL;
    }

    inst = *code_buf;
    cpu->pc = addr + 4;
    if(inst == 0xACBF011C) {
        prev_ra = cpu->gpr[MREG_RA].d;
        cpu->gpr[MREG_RA].w[1] = cpu->n64_ra;
        save_ra = 1;
    }

    switch(MIPS_OP(inst)) {
        case OPC_SPECIAL:
            switch(SPEC_FUNCT(inst)) {
                case SPEC_SLL: // SLL
                    cpu->gpr[MIPS_RD(inst)].w[1] = cpu->gpr[MIPS_RT(inst)].w[1] << MIPS_SA(inst);
                    break;
                case SPEC_SRL: // SRL
                    cpu->gpr[MIPS_RD(inst)].w[1] = cpu->gpr[MIPS_RT(inst)].w[1] >> MIPS_SA(inst);
                    break;
                case SPEC_SRA: // SRA
                    cpu->gpr[MIPS_RD(inst)].sw[1] = cpu->gpr[MIPS_RT(inst)].sw[1] >> MIPS_SA(inst);
                    break;
                case SPEC_SLLV: // SLLV
                    cpu->gpr[MIPS_RD(inst)].w[1] = 
                        cpu->gpr[MIPS_RT(inst)].w[1] << (cpu->gpr[MIPS_RS(inst)].w[1] & 0x1F);
                    break;
                case SPEC_SRLV: // SRLV
                    cpu->gpr[MIPS_RD(inst)].w[1] = 
                        cpu->gpr[MIPS_RT(inst)].w[1] >> (cpu->gpr[MIPS_RS(inst)].w[1] & 0x1F);
                    break;
                case SPEC_SRAV: // SRAV
                    cpu->gpr[MIPS_RD(inst)].sw[1] = 
                        cpu->gpr[MIPS_RT(inst)].sw[1] >> (cpu->gpr[MIPS_RS(inst)].sw[1] & 0x1F);
                    break;
                case SPEC_JR: // JR
                    cpu->unk_0x24 = cpu->gpr[MIPS_RS(inst)].w[1];
                    break;
                case SPEC_JALR: // JALR
                    cpu->unk_0x24 = cpu->gpr[MIPS_RS(inst)].w[1];
                    cpu->gpr[MIPS_RD(inst)].d = cpu->pc + 4;
                    break;
                case SPEC_SYSCALL: // SYSCALL
                    cpuException(cpu, 8, 0);
                    break;
                case SPEC_BREAK: // BREAk
                    cpuException(cpu, 9, 0);
                    break;
                case SPEC_MFHI: // MFHI
                    cpu->gpr[MIPS_RD(inst)].d = cpu->hi.d;
                    break;
                case SPEC_MTHI: // MTHI
                    cpu->hi.d = cpu->gpr[MIPS_RS(inst)].d;
                    break;
                case SPEC_MFLO: // MFLO
                    cpu->gpr[MIPS_RD(inst)].d = cpu->lo.d;
                    break;
                case SPEC_MTLO: // MTLO
                    cpu->lo.d = cpu->gpr[MIPS_RS(inst)].d;
                    break;
                case SPEC_DSLLV: // DSLLV
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RT(inst)].d << (cpu->gpr[MIPS_RS(inst)].d & 0x3F);
                    break;
                case SPEC_DSRLV: // DSRLV
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RT(inst)].d >> (cpu->gpr[MIPS_RS(inst)].d & 0x3F);
                    break;
                case SPEC_DSRAV: // DRAV
                    cpu->gpr[MIPS_RD(inst)].sd = cpu->gpr[MIPS_RT(inst)].sd >> (cpu->gpr[MIPS_RS(inst)].sd & 0x3F);
                    break;
                case SPEC_MULT: // MULT
                    {
                        tmp.d = (s64)((s64)cpu->gpr[MIPS_RS(inst)].sw[1] * (s64)cpu->gpr[MIPS_RT(inst)].sw[1]);
                        cpu->lo.sd = (s32)(tmp.sd & 0xFFFFFFFF);
                        cpu->hi.sd = (s32)(tmp.sd >> 32);
                    }
                    break;
                case SPEC_MULTU: // MULTU
                    {
                        tmp.d = (s64)((u64)cpu->gpr[MIPS_RS(inst)].w[1] * (u64)cpu->gpr[MIPS_RT(inst)].w[1]);
                        cpu->lo.sd = (s32)(tmp.sd & 0xFFFFFFFF);
                        cpu->hi.sd = (s32)(tmp.sd >> 32);
                    }
                    break;
                case SPEC_DIV: // DIV
                    {
                        if(cpu->gpr[MIPS_RT(inst)].w[1] == 0) {
                            break;
                        }

                        cpu->lo.sd = cpu->gpr[MIPS_RS(inst)].sw[1] / cpu->gpr[MIPS_RT(inst)].sw[1];

                        cpu->hi.sd = cpu->gpr[MIPS_RS(inst)].sw[1] % cpu->gpr[MIPS_RT(inst)].sw[1];
                    }
                    break;
                case SPEC_DIVU: // DIVU
                    {
                        if(cpu->gpr[MIPS_RT(inst)].w[1] == 0) {
                            break;
                        }

                        cpu->lo.sd = (s32)(cpu->gpr[MIPS_RS(inst)].w[1] / cpu->gpr[MIPS_RT(inst)].w[1]);
                        cpu->hi.sd = (s32)(cpu->gpr[MIPS_RS(inst)].w[1] % cpu->gpr[MIPS_RT(inst)].w[1]);
                    }
                    break;

                case SPEC_DMULT: // DMULT
                    {
                        cpu->lo.d = cpu->gpr[MIPS_RS(inst)].d * cpu->gpr[MIPS_RT(inst)].d;
                        cpu->hi.sd = -(cpu->lo.sd < 0);
                    }
                    break;
                case SPEC_DMULTU: // DMULTU
                    {
                        cpu->lo.d = cpu->gpr[MIPS_RS(inst)].d * cpu->gpr[MIPS_RT(inst)].d;
                        cpu->hi.sd = -(cpu->lo.sd < 0);
                    }
                    break;
                case SPEC_DDIV: // DDIV
                    if(cpu->gpr[MIPS_RT(inst)].d == 0) {
                        break;
                    }

                    cpu->lo.sd = cpu->gpr[MIPS_RS(inst)].sd / cpu->gpr[MIPS_RT(inst)].sd;
                    cpu->hi.sd = cpu->gpr[MIPS_RS(inst)].sd % cpu->gpr[MIPS_RT(inst)].sd;
                    break;

                case SPEC_DDIVU: // DDIVU
                    if(cpu->gpr[MIPS_RT(inst)].d == 0) {
                        break;
                    }

                    cpu->lo.d = cpu->gpr[MIPS_RS(inst)].d / cpu->gpr[MIPS_RT(inst)].d;
                    cpu->hi.d = cpu->gpr[MIPS_RS(inst)].d % cpu->gpr[MIPS_RT(inst)].d;
                    break;
                case SPEC_ADD: // ADD
                    cpu->gpr[MIPS_RD(inst)].sw[1] = cpu->gpr[MIPS_RS(inst)].sw[1] + cpu->gpr[MIPS_RT(inst)].sw[1];
                    break;
                case SPEC_ADDU: // ADDU
                    cpu->gpr[MIPS_RD(inst)].w[1] = cpu->gpr[MIPS_RS(inst)].w[1] + cpu->gpr[MIPS_RT(inst)].w[1];
                    break;
                case SPEC_SUB:  // SUB
                    cpu->gpr[MIPS_RD(inst)].sw[1] = cpu->gpr[MIPS_RS(inst)].sw[1] - cpu->gpr[MIPS_RT(inst)].sw[1];
                    break;
                case SPEC_SUBU: // SUBU
                    cpu->gpr[MIPS_RD(inst)].sw[1] = cpu->gpr[MIPS_RS(inst)].sw[1] - cpu->gpr[MIPS_RT(inst)].sw[1];
                    break;
                case SPEC_AND: // AND
                    cpu->gpr[MIPS_RD(inst)].w[1] = cpu->gpr[MIPS_RS(inst)].w[1] & cpu->gpr[MIPS_RT(inst)].w[1];
                    break;
                case SPEC_OR: // OR
                    cpu->gpr[MIPS_RD(inst)].w[1] = cpu->gpr[MIPS_RS(inst)].w[1] | cpu->gpr[MIPS_RT(inst)].w[1];
                    break;
                case SPEC_XOR: // XOR
                    cpu->gpr[MIPS_RD(inst)].w[1] = cpu->gpr[MIPS_RS(inst)].w[1] ^ cpu->gpr[MIPS_RT(inst)].w[1];
                    break;
                case SPEC_NOR: // NOR
                    cpu->gpr[MIPS_RD(inst)].w[1] = ~(cpu->gpr[MIPS_RS(inst)].w[1] | cpu->gpr[MIPS_RT(inst)].w[1]);
                    break;
                case SPEC_SLT: // SLT
                    cpu->gpr[MIPS_RD(inst)].sw[1] = !!(cpu->gpr[MIPS_RS(inst)].sw[1] < cpu->gpr[MIPS_RT(inst)].sw[1]);
                    break;
                case SPEC_SLTU: // SLTU
                    cpu->gpr[MIPS_RD(inst)].w[1] = !!(cpu->gpr[MIPS_RS(inst)].w[1] < cpu->gpr[MIPS_RT(inst)].w[1]);
                    break;
                case SPEC_DADD: // DADD
                    cpu->gpr[MIPS_RD(inst)].sd = cpu->gpr[MIPS_RS(inst)].sd + cpu->gpr[MIPS_RT(inst)].sd;
                    break;
                case SPEC_DADDU: // DADDU
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RS(inst)].d + cpu->gpr[MIPS_RT(inst)].d;
                    break;
                case SPEC_DSUB: // DSUB
                    cpu->gpr[MIPS_RD(inst)].sd = cpu->gpr[MIPS_RS(inst)].sd - cpu->gpr[MIPS_RT(inst)].sd;
                    break;
                case SPEC_DSUBU: // DSUBU
                    cpu->gpr[MIPS_RD(inst)].sd = cpu->gpr[MIPS_RS(inst)].sd - cpu->gpr[MIPS_RT(inst)].sd;
                    break;
                case SPEC_TGE: // TGE
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] >= cpu->gpr[MIPS_RT(inst)].sw[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case SPEC_TGEU: // TGEU
                    if(cpu->gpr[MIPS_RS(inst)].w[1] >= cpu->gpr[MIPS_RT(inst)].w[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case SPEC_TLT: // TGE
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] < cpu->gpr[MIPS_RT(inst)].sw[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case SPEC_TLTU: // TGEU
                    if(cpu->gpr[MIPS_RS(inst)].w[1] < cpu->gpr[MIPS_RT(inst)].w[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case SPEC_TEQ: // TEQ
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] == cpu->gpr[MIPS_RT(inst)].sw[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case SPEC_TNE: // TNE
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] != cpu->gpr[MIPS_RT(inst)].sw[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case SPEC_DSLL: // DSLL
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RT(inst)].d << MIPS_SA(inst);
                    break;
                case SPEC_DSRL: // DSRL
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RT(inst)].d >> MIPS_SA(inst);
                    break;
                case SPEC_DSRA: // DSRA
                    cpu->gpr[MIPS_RD(inst)].sd = cpu->gpr[MIPS_RT(inst)].sd >> MIPS_SA(inst);
                    break;
                case SPEC_DSLL32: // DSLL32
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RT(inst)].d << (MIPS_SA(inst) + 32);
                    break;
                case SPEC_DSRL32: // DSRL32
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RT(inst)].d >> (MIPS_SA(inst) + 32);
                    break;
                case SPEC_DSRA32: // DSRA32
                    cpu->gpr[MIPS_RD(inst)].sd = cpu->gpr[MIPS_RT(inst)].sd >> (MIPS_SA(inst) + 32);
                    break;

            }       
            break;
        case OPC_REGIMM:
            switch(REGIMM_SUB(inst)) {
                case REGIMM_BLTZ: // BLTZ
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] < 0) {
                        cpu->unk_0x24 = cpu->pc + (MIPS_IMM(inst) * 4);
                    }
                    break;
                case REGIMM_BGEZ: // BGEZ
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] >= 0) {
                        cpu->unk_0x24 = cpu->pc + (MIPS_IMM(inst) * 4);
                    }
                    break;
                case REGIMM_BLTZL: // BLTZL
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] < 0) {
                        cpu->unk_0x24 = cpu->pc + (MIPS_IMM(inst) * 4);
                    } else {
                        cpu->status |= 4;
                        cpu->pc += 4;
                    }
                    break;
                case REGIMM_BGEZL: // BGEZL
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] >= 0) {
                        cpu->unk_0x24 = cpu->pc + (MIPS_IMM(inst) * 4);
                    } else {
                        cpu->status |= 4;
                        cpu->pc += 4;
                    }
                    break;
                case REGIMM_TGEI: // TGEI
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] >= MIPS_IMM(inst) ) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case REGIMM_TGEIU: // TGEIU
                    if(cpu->gpr[MIPS_RS(inst)].w[1] >= MIPS_IMM(inst) ) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case REGIMM_TLTI: // TLTI
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] < MIPS_IMM(inst) ) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case REGIMM_TLTIU: // TLTIU
                    if(cpu->gpr[MIPS_RS(inst)].w[1] < MIPS_IMM(inst) ) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case REGIMM_TEQI: // TEQI
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] == MIPS_IMM(inst) ) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case REGIMM_TNEI: // TNEI
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] != MIPS_IMM(inst) ) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case REGIMM_BLTZAL: // BLTZAL
                    cpu->gpr[0x1F].w[1] = cpu->pc + 4;
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] < 0) {
                        cpu->unk_0x24 = cpu->unk_0x28 = cpu->pc + (MIPS_IMM(inst) * 4);
                    }
                    break;
                case REGIMM_BGEZAL: // BGEZAL
                    cpu->gpr[0x1F].w[1] = cpu->pc + 4;
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] >= 0) {
                        cpu->unk_0x24 = cpu->unk_0x28 = cpu->pc + (MIPS_IMM(inst) * 4);
                    }
                    break;
                case REGIMM_BLTZALL: // BLTZALL
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] < 0) {
                        cpu->gpr[0x1F].sw[1] = cpu->pc + 4;
                        cpu->unk_0x24 = cpu->pc + (MIPS_IMM(inst) * 4);
                    } else {
                        cpu->status |= 4;
                        cpu->pc += 4;
                    }
                    break;
                case REGIMM_BGEZALL: // BGEZALL
                    if(cpu->gpr[MIPS_RS(inst)].sw[1] >= 0) {
                        cpu->gpr[0x1F].sw[1] = cpu->pc + 4;
                        cpu->unk_0x24 = cpu->pc + (MIPS_IMM(inst) * 4);
                    } else {
                        cpu->status |= 4;
                        cpu->pc += 4;
                    }
                    break;
            }
            break;
        case OPC_J: // J
            {
                cpu->unk_0x24 = (cpu->pc & 0xF0000000) | ((inst & 0x3FFFFFF) << 2);
                if(cpu->unk_0x24 != cpu->pc - 4) {
                    break;
                }

                if(!checkRetrace()) {
                    return NULL;
                }
            }
            break;
        case OPC_JAL: //JAL
            {
                recomp_node_t *node;
    
                cpu->gpr[MREG_RA].w[1] = cpu->pc + 4;
                cpu->unk_0x24 = cpu->unk_0x28 = (cpu->pc & 0xF0000000) | ((inst & 0x3FFFFFF) << 2);
                cpuFindFunction(cpu, cpu->unk_0x24, &node);
            }
            break;
        case OPC_BEQ: // BEQ
             if(cpu->gpr[MIPS_RS(inst)].sw[1] == cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                 cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
             }

             if(cpu->unk_0x24 != cpu->pc - 4) {
                 break;
             }

             if(!checkRetrace()) {
                 return NULL;
             }
             break;
        case OPC_BNE: // BNE
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] != cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
            }
            break;
        case OPC_BLEZ: // BLEZ
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] <= 0) {
                cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
            }
            break;
        case OPC_BGTZ: // BGTZ
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] > 0) {
                cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
            }
            break;
        case OPC_ADDI: // ADDI
            cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst) ;
            break;
        case OPC_ADDIU: // ADDIU
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst) ;
            break;
        case OPC_SLTI: // SLTI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = !!(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < MIPS_IMM(inst) );
            break;
        case OPC_SLTIU: // SLTIU
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = !!(cpu->gpr[(inst >> 0x15) & 0x1F].w[1] < MIPS_IMM(inst) );
            break;
        case OPC_ANDI: // ANDI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] & (inst & 0xFFFF);
            break;
        case OPC_ORI: // ORI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] | (inst & 0xFFFF);
            break;
        case OPC_XORI: // XORI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] ^ (inst & 0xFFFF);
            break; 
        case OPC_LUI: // LUI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = (inst & 0xFFFF) << 0x10;
            
            break;
        case OPC_CP0: // CP0
            switch(inst & 0x3F) {
                case 1: // TLBR
                    cpu->cp0[CP0_ENTRYLO0].d = cpu->tlb[cpu->cp0[CP0_INDEX].w[1] & 0x3F].entry_lo0.d;
                    cpu->cp0[CP0_ENTRYLO1].d = cpu->tlb[cpu->cp0[CP0_INDEX].w[1] & 0x3F].entry_lo1.d;
                    cpu->cp0[CP0_ENTRYHI].d = cpu->tlb[cpu->cp0[CP0_INDEX].w[1] & 0x3F].entry_hi.d;
                    cpu->cp0[CP0_PAGEMASK].d = cpu->tlb[cpu->cp0[CP0_INDEX].w[1] & 0x3F].page_mask.d;
                    break;
                case 2: // TLBWI
                    cpuSetTLB(cpu, cpu->cp0[CP0_INDEX].w[1] & 0x3F);
                    break;
                case 5: // TLBWR
                    {
                        setCP0Random(cpu);
                    }
                    break;
                case 8: // TLBP
                    {
                        cpu->cp0[CP0_INDEX].d |= 0x80000000;

                        for(i = 0; i < 48; i++) {
                            if(cpu->tlb[i].entry_lo0.w[1] & 2) {
                                if(!(cpu->cp0[CP0_ENTRYHI].d ^ cpu->tlb[i].entry_hi.d)) {
                                    cpu->cp0[CP0_INDEX].d = i;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case 24: // ERET
                    if(cpu->cp0[12].d & 4) {
                        cpu->pc = cpu->cp0[CP0_ERROREPC].w[1];
                        cpu->cp0[CP0_STATUS].d &= ~STATUS_ERL;
                    } else {
                        cpu->pc = cpu->cp0[CP0_EPC].w[1];
                        cpu->cp0[CP0_STATUS].d &= ~STATUS_EXL;
                    }
                    cpu->status |= 0x24;
                    break;
                case 0:
                default:
                    switch((inst >> 0x15) & 0x1F) {
                        case 0: // MFC0
                            {
                                if(!cpuGetRegisterCP0(cpu, (inst >> 0xB) & 0x1F, &tmp.d)) {
                                    break;
                                }

                                cpu->gpr[(inst >> 0x10) & 0x1F].sd = tmp.sd & 0xFFFFFFFFULL;
                            }
                            break;
                        case 1: // DMFC0
                            {
                                if(!cpuGetRegisterCP0(cpu, (inst >> 0xB) & 0x1F, &tmp.d)) {
                                    break;
                                }

                                cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.d;
                            }
                            break;
                        case 2:
                            break;
                        case 4: // MTC0
                            cpuSetRegisterCP0(cpu, (inst >> 0xB) & 0x1F, cpu->gpr[(inst >> 0x10) & 0x1F].w[1]);
                            break;
                        case 5: // DMTC0
                            cpuSetRegisterCP0(cpu, (inst >> 0xB) & 0x1F, cpu->gpr[(inst >> 0x10) & 0x1F].d);
                            break;
                        case 8:
                            break;
                    }
                    break;
            }
            break;
        case OPC_CP1:
            if(MIPS_FDT(inst) == 0) {
                if(MIPS_FSUB(inst) < 0x10) {
                    switch(MIPS_FSUB(inst)) {
                        case MIPS_FSUB_MFC:
                            if(MIPS_FS(inst) & 1) {
                                cpu->gpr[MIPS_RT(inst)].w[1] = cpu->fpr[MIPS_FS(inst) - 1].w[0];
                            } else {
                                cpu->gpr[MIPS_RT(inst)].w[1] = cpu->fpr[MIPS_FS(inst)].w[1];
                            }
                            break;
                        case MIPS_FSUB_DMFC:
                            cpu->gpr[MIPS_RT(inst)].d = cpu->fpr[MIPS_FS(inst)].d;
                            break;
                        case MIPS_FSUB_CFC:
                            cpu->gpr[MIPS_RT(inst)].w[1] = cpu->fscr[MIPS_FS(inst)];
                            break;
                        case MIPS_FSUB_MTC:
                            if(MIPS_FS(inst) & 1) {
                                cpu->fpr[MIPS_FS(inst) - 1].d = cpu->fpr[MIPS_FS(inst) - 1].d & 0xFFFFFFFFUL;
                                cpu->fpr[MIPS_FS(inst) - 1].d |= ((u64)cpu->gpr[MIPS_RT(inst)].w[1] << 32);

                            } else {
                                cpu->fpr[MIPS_FS(inst)].w[1] = cpu->gpr[MIPS_RT(inst)].w[1];
                            }
                            break;
                        case MIPS_FSUB_DMTC:
                            cpu->fpr[MIPS_FS(inst)].d = cpu->gpr[MIPS_RT(inst)].d;
                            break;
                        case MIPS_FSUB_CTC:
                            cpu->fscr[MIPS_FS(inst)] = cpu->gpr[MIPS_RT(inst)].w[1];
                            break;
                    }
                    break;
                }
            }
            
            // Condition Code
            if(MIPS_FFMT(inst) == 8) {
                switch((inst >> 0x10) & 0x1F) {
                    case 0: // BC1F
                        if(!(cpu->fscr[0x1F] & 0x800000)) {
                            cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
                        }
                        break;
                    case 1: // BC1T
                        if(cpu->fscr[0x1F] & 0x800000) {
                            cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
                        }
                        break;
                    case 2: // BC1FL
                        if(!(cpu->fscr[0x1F] & 0x800000)) {
                            cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
                        } else {
                            cpu->status |= 4;
                            cpu->pc += 4;
                        }
                        break;
                    case 3: // BC1TL
                        if(cpu->fscr[0x1F] & 0x800000) {
                            cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
                        } else {
                            cpu->status |= 4;
                            cpu->pc += 4;
                        }
                        break;
                }
                break;
            }

            switch(MIPS_FFMT(inst)) {
                case MIPS_FMT_SINGLE:
                    switch(MIPS_FFUNC(inst)) {
                        case 0: // ADD.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].f[1] + cpu->fpr[(inst >> 0x10) & 0x1F].f[1];
                            break;
                        case 1: // SUB.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].f[1] - cpu->fpr[(inst >> 0x10) & 0x1F].f[1];
                            break;
                        case 2: // MUL.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].f[1] * cpu->fpr[(inst >> 0x10) & 0x1F].f[1];
                            break;
                        case 3: // DIV.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].f[1] / cpu->fpr[(inst >> 0x10) & 0x1F].f[1];
                            break;
                        case 4: // SQRT.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = sqrt(cpu->fpr[(inst >> 0xB) & 0x1F].f[1]);
                            break;
                        case 5: // ABS.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = __fabs(cpu->fpr[(inst >> 0xB) & 0x1F].f[1]);
                            break;
                        case 6: // MOV.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 7: // NEG.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = -cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 8: // ROUND.L.S
                            cpu->fpr[(inst >> 6) & 0x1F].sd = 0.5f + cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 9: // TRUNC.L.S
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 10: // CEIL.L.S
                            cpu->fpr[(inst >> 6) & 0x1F].sd = ceil(cpu->fpr[(inst >> 0xB) & 0x1F].f[1]);
                            break;
                        case 11: // FLOOR.L.S
                            cpu->fpr[(inst >> 6) & 0x1F].sd = floor(cpu->fpr[(inst >> 0xB) & 0x1F].f[1]);
                            break;
                        case 12: // ROUND.W.S
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = 0.5f + cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 13: // TRUNC.W.S
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 14: // CEIL.W.S
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = ceil(cpu->fpr[(inst >> 0xB) & 0x1F].f[1]);
                            break;
                        case 15: // FLOOR.W.S
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = floor(cpu->fpr[(inst >> 0xB) & 0x1F].f[1]);
                            break;
                        case 32: // CVT.S.S
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 33: // CVT.D.S
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 34:
                        case 35:
                            break;
                        case 37: // CVT.W.S
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 38: // CVT.L.S
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].f[1];
                            break;
                        case 48: // C.F.S
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 49: // C.UN.S
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 50: // C.EQ.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] == cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 51: // C.UEQ.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] == cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 52: // C.OLT.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] < cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 53: // C.ULT.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] < cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 54: // C.OLE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 55: // C.ULE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 56: // C.SF.S
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 57: // C.NGLE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 58: // C.SEQ.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] == cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 59: // C.NGL.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] == cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 60: // C.LT.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] < cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 61: // C.NGE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] < cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 62:  // C.LE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 63: // C.NGT.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                    }
                    break;
                case MIPS_FMT_DOUBLE:
                    switch(MIPS_FFUNC(inst)) {
                        case 0: // ADD.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].fd + cpu->fpr[(inst >> 0x10) & 0x1F].fd;
                            break;
                        case 1: // SUB.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].fd - cpu->fpr[(inst >> 0x10) & 0x1F].fd;
                            break;
                        case 3: // MUL.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].fd * cpu->fpr[(inst >> 0x10) & 0x1F].fd;
                            break;
                        case 4: // DIV.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].fd / cpu->fpr[(inst >> 0x10) & 0x1F].fd;
                            break;
                        case 5: // SQRT.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = sqrt(cpu->fpr[(inst >> 0xB) & 0x1F].fd);
                            break;
                        case 6: // ABS.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = __fabs(cpu->fpr[(inst >> 0xB) & 0x1F].fd);
                            break;
                        case 7: // MOV.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 8: // NEG.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = -cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 9: // ROUND.L.D
                            cpu->fpr[(inst >> 6) & 0x1F].sd = 0.5 + cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 10: // TRUNC.L.D
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 11: // CEIL.L.D
                            cpu->fpr[(inst >> 6) & 0x1F].sd = ceil(cpu->fpr[(inst >> 0xB) & 0x1F].fd);
                            break;
                        case 12: // FLOOR.L.D
                            cpu->fpr[(inst >> 6) & 0x1F].sd = floor(cpu->fpr[(inst >> 0xB) & 0x1F].fd);
                            break;
                        case 13: // ROUND.W.D
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = 0.5 + cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 14: // TRUNC.W.D
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 15: // CEIL.W.D
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = ceil(cpu->fpr[(inst >> 0xB) & 0x1F].fd);
                            break;
                        case 16: // FLOOR.W.D
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = floor(cpu->fpr[(inst >> 0xB) & 0x1F].fd);
                            break;
                        case 33: // CVT.S.D
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 34: // CVT.D.D
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 37: // CVT.W.D
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 38: // CVT.L.D
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].fd;
                            break;
                        case 48: // C.F.D
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 49: // C.UN.D
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 50: // C.EQ.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd == cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 51: // C.UEQ.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd == cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 52: // C.OLT.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd < cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 53: // C.ULT.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd < cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 54: // C.OLE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 55: // C.ULE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 56: // C.SF.D
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 57: // C.NGLE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 58: // C.SEQ.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd == cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 59: // C.NGL.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd == cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 60: // C.LT.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd < cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 61: // C.NGE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd < cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 62:  // C.LE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 63: // C.NGT.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                    }
                    break;
                case MIPS_FMT_WORD:
                    switch(MIPS_FFUNC(inst)) {
                        case 0: // ADD.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] + cpu->fpr[(inst >> 0x10) & 0x1F].sw[1];
                            break;
                        case 1: // SUB.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] - cpu->fpr[(inst >> 0x10) & 0x1F].sw[1];
                            break;
                        case 2: // MUL.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] * cpu->fpr[(inst >> 0x10) & 0x1F].sw[1];
                            break;
                        case 3: // DIV.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] / cpu->fpr[(inst >> 0x10) & 0x1F].sw[1];
                            break;
                        case 4: // SQRT.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = sqrt(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1]);
                            break;
                        case 5: // ABS.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = __fabs(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1]);
                            break;
                        case 6: // MOV.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 7: // NEG.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = -cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 8: // ROUND.L.W
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 9: // TRUNC.L.W
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 10: // CEIL.L.W
                            cpu->fpr[(inst >> 6) & 0x1F].sd = ceil(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1]);
                            break;
                        case 11: // FLOOR.L.W
                            cpu->fpr[(inst >> 6) & 0x1F].sd = floor(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1]);
                            break;
                        case 12: // ROUND.W.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 13: // TRUNC.W.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 14: // CEIL.W.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = ceil(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1]);
                            break;
                        case 15: // FLOOR.W.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = floor(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1]);
                            break;
                        case 32: // CVT.S.W
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 33: // CVT.D.W
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 37: // CVT.W.W
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 38: // CVT.L.W
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sw[1];
                            break;
                        case 48: // C.F.W
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 49: // C.UN.W
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 50: // C.EQ.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] == cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 51: // C.UEQ.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] == cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 52: // C.OLT.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] < cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 53: // C.ULT.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] < cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 54: // C.OLE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 55: // C.ULE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 56: // C.SF.W
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 57: // C.NGLE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 58: // C.SEQ.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] == cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 59: // C.NGL.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] == cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 60: // C.LT.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] < cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 61: // C.NGE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] < cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 62:  // C.LE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 63: // C.NGT.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                    }
                    break;
                case MIPS_FMT_DBLWORD:
                    switch(MIPS_FFUNC(inst)) {
                        case 0: // ADD.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sd + cpu->fpr[(inst >> 0x10) & 0x1F].sd;
                            break;
                        case 1: // SUB.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sd - cpu->fpr[(inst >> 0x10) & 0x1F].sd;
                            break;
                        case 2: // MUL.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sd * cpu->fpr[(inst >> 0x10) & 0x1F].sd;
                            break;
                        case 3: // DIV.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sd / cpu->fpr[(inst >> 0x10) & 0x1F].sd;
                            break;
                        case 4: // SQRT.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = sqrt(cpu->fpr[(inst >> 0xB) & 0x1F].sd);
                            break;
                        case 5: // ABS.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = __fabs(cpu->fpr[(inst >> 0xB) & 0x1F].sd);
                            break;
                        case 6: // MOV.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 7: // NEG.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = -cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 8: // ROUND.L.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 9: // TRUNC.L.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 10: // CEIL.L.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = ceil(cpu->fpr[(inst >> 0xB) & 0x1F].sd);
                            break;
                        case 11: // FLOOR.L.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = floor(cpu->fpr[(inst >> 0xB) & 0x1F].sd);
                            break;
                        case 12: // ROUND.W.L
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 13: // TRUNC.W.L
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 14: // CEIL.W.L
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = ceil(cpu->fpr[(inst >> 0xB) & 0x1F].sd);
                            break;
                        case 15: // FLOOR.W.L
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = floor(cpu->fpr[(inst >> 0xB) & 0x1F].sd);
                            break;
                        case 32: // CVT.S.L
                            cpu->fpr[(inst >> 6) & 0x1F].f[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 33: // CVT.D.L
                            cpu->fpr[(inst >> 6) & 0x1F].fd = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 37: // CVT.W.L
                            cpu->fpr[(inst >> 6) & 0x1F].sw[1] = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 38: // CVT.L.L
                            cpu->fpr[(inst >> 6) & 0x1F].sd = cpu->fpr[(inst >> 0xB) & 0x1F].sd;
                            break;
                        case 48: // C.F.L
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 49: // C.UN.L
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 50: // C.EQ.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd == cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 51: // C.UEQ.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd == cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 52: // C.OLT.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd < cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 53: // C.ULT.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd < cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 54: // C.OLE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 55: // C.ULE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 56: // C.SF.L
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 57: // C.NGLE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 58: // C.SEQ.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd == cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 59: // C.NGL.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd == cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 60: // C.LT.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd < cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 61: // C.NGE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd < cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 62:  // C.LE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 63: // C.NGT.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                    }
                    break;
            }
            break;
        case 20: // BEQL
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] == cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
            } else {
                cpu->status |= 4;
                cpu->pc += 4;
            }
            break;
        case 21: // BNEL
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] != cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
            } else {
                cpu->status |= 4;
                cpu->pc += 4;
            }
            break;
        case 22: // BLEZL
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] <= 0) {
                cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
            } else {
                cpu->status |= 4;
                cpu->pc += 4;
            }
            break;
        case 23: // BGTZL
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] > 0) {
                cpu->unk_0x24 = cpu->pc + MIPS_IMM(inst)  * 4;
            } else {
                cpu->status |= 4;
                cpu->pc += 4;
            }
            break;
        case 24: // DADDI
            cpu->gpr[(inst >> 0x10) & 0x1F].sd = cpu->gpr[(inst >> 0x15) & 0x1F].sd + MIPS_IMM(inst) ;
            break;
        case 25: // DADDIU
            cpu->gpr[(inst >> 0x10) & 0x1F].d = cpu->gpr[(inst >> 0x15) & 0x1F].d + MIPS_IMM(inst);
            break;
        case 31: // precompiled function.
            if(!func_8005D614(gSystem->unk_0x0058, cpu, MIPS_IMM(inst) )) {
                return NULL;
            }
            break;
        case 26: // LDL
            {
                sh = 0x38;
                ea = cpu->gpr[MIPS_RS(inst)].sd + MIPS_IMM(inst);
                do {
                    dev = get_dev(devs, dev_idx, ea);
                    if(dev->lb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf)) {
                        tmp.d = (u64)byteBuf << (u64)(sh);
                        cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.d | (cpu->gpr[(inst >> 0x10) & 0x1F].d & ~((u64)0xFF << (sh)));
                    }
                    
                    ea++;
                    sh -= 8;

                } while(((ea - 1) & 7));
            }
            break;
        case 27: // LDR
            {
                sh = 0;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sd + MIPS_IMM(inst);

                do {
                    if(devs[dev_idx[ea >> 0x10]]->lb(devs[dev_idx[ea >> 0x10]]->dev_obj, ea + devs[dev_idx[ea >> 0x10]]->addr_offset, (s8*)&byteBuf)) {
                        tmp.d = (u64)byteBuf << sh;
                        cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.d | (cpu->gpr[(inst >> 0x10) & 0x1F].d & ~((u64)0xFF << sh));
                    }
                    ea--;
                    sh += 8;
                } while((ea + 1) & 7);
            }
            break;
        case 39: // LWU
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sd + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lw(dev->dev_obj, ea + dev->addr_offset, (s32*)&wordBuf)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].d = wordBuf;
                }
            }
            break;
        case 32: // LB
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->lb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = (s8)byteBuf;
                }
            }
            break;
        case 33: // LH
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->lh(dev->dev_obj, ea + dev->addr_offset, (s16*)&halfBuf)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = (s16)halfBuf;
                }
            }
            break;
        case 34: // LWL
            {
                sh = 0x18;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    dev = get_dev(devs, dev_idx, ea);
                    if(dev->lb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf)) {
                        wordBuf = (u32)byteBuf << sh;
                        cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = wordBuf | (cpu->gpr[(inst >> 0x10) & 0x1F].w[1] & ~((u32)0xFF << sh));
                    }
                    ea++;
                    sh -= 8;
                } while((ea - 1) & 3);
            }
            break;
        case 35: // LW
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->lw(dev->dev_obj, ea + dev->addr_offset, (s32*)&wordBuf)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = wordBuf;
                }
            }
            break;
        case 36: // LBU
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->lb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = byteBuf;
                }
            }
            break;
        case 37: // LHU
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);

                if(func_80052D68(gSystem->frame, &halfBuf, ea)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = halfBuf;
                    break;
                }
                
                dev = get_dev(devs, dev_idx, ea);

                if(dev->lh(dev->dev_obj, ea + dev->addr_offset, (s16*)&halfBuf)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = halfBuf;
                }
            }
            break;
        case 38: // LWR
            {
                sh = 0;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    dev = get_dev(devs, dev_idx, ea);
                    if(dev->lb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf)) {
                        wordBuf = (u32)byteBuf << sh;
                        cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = wordBuf | (cpu->gpr[(inst >> 0x10) & 0x1F].w[1] & ~((u32)0xFF << sh));
                    }
                    ea--;
                    sh += 8;
                } while((ea + 1) & 3);
            }
            break;
        case 40: // SB
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                dev->sb(dev->dev_obj, ea + dev->addr_offset, (s8*)&cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]);
            }
            break;
        case 41: // SH
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                dev->sh(dev->dev_obj, ea + dev->addr_offset, (s16*)&cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]);
            }
            break;
        case 42: // SWL
            {
                sh = 0x18;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    byteBuf = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] >> sh;
                    dev = get_dev(devs, dev_idx, ea);
                    dev->sb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf);
                    ea++;
                    sh -= 8;
                } while((ea - 1) & 3);
            }
            break;
        case 43: // SW
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                dev->sw(dev->dev_obj, ea + dev->addr_offset, &cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]);
            }
            break;
        case 44: // SDL
            {
                sh = 0x38;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sd + MIPS_IMM(inst);

                do {
                    byteBuf = cpu->gpr[(inst >> 0x10) & 0x1F].d >> sh;
                    dev = get_dev(devs, dev_idx, ea);
                    dev->sb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf);
                    ea++;
                    sh -= 8;
                } while((ea - 1) & 7);
            }
            break;
        case 45: // SDR
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                sh = 0;

                do {
                    byteBuf = cpu->gpr[(inst >> 0x10) & 0x1F].d >> sh;
                    dev = get_dev(devs, dev_idx, ea);
                    dev->sb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf);
                    ea--;
                    sh += 8;
                } while((ea + 1) & 7);
            }
            break;
        case 46: // SWR
            {
                sh = 0;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    byteBuf = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] >> sh;
                    dev = get_dev(devs, dev_idx, ea);
                    dev->sb(dev->dev_obj, ea + dev->addr_offset, (s8*)&byteBuf);
                    ea--;
                    sh += 8;
                } while((ea + 1) & 3);
            }
            break;
        case 48: // LL
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->lw(dev->dev_obj, ea + dev->addr_offset, (s32*)&wordBuf)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = wordBuf;
                }
            }
            break;
        case 49: // LWC1
            {
                ea = cpu->gpr[MIPS_RS(inst)].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->lw(dev->dev_obj, ea + dev->addr_offset, (s32*)&wordBuf)) {
                    if(MIPS_RT(inst) & 1) {

                        cpu->fpr[MIPS_FT(inst) - 1].d = cpu->fpr[MIPS_FT(inst) - 1].d & 0xFFFFFFFF;
                        cpu->fpr[MIPS_FT(inst) - 1].d |= ((u64)wordBuf << 32);
                    } else {
                        cpu->fpr[(inst >> 0x10) & 0x1F].sw[1] = wordBuf;
                    }
                }
            }
            break;
        case 52: // LLD
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sd + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->ld(dev->dev_obj, ea + dev->addr_offset, &tmp.sd)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.sd;
                }
            }
            break;
        case 53: // LDC1 
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->ld(dev->dev_obj, ea + dev->addr_offset, &tmp.sd)) {
                    cpu->fpr[(inst >> 0x10) & 0x1F].d = tmp.sd;
                }
            }
            break;
        case 55: // LD
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                if(dev->ld(dev->dev_obj, ea + dev->addr_offset, &tmp.sd)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.sd;
                }
            }
            break;
        case 56: // SC
            {
                wordBuf = cpu->gpr[MIPS_RT(inst)].w[1];
                ea = cpu->gpr[MIPS_RS(inst)].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);
                
                cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = !!dev->sw(dev->dev_obj, ea + dev->addr_offset, (s32*)&wordBuf);
            }
            break;
        case 57: // SWC1
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);

                if(MIPS_RT(inst) & 1) {
                    wordBuf = cpu->fpr[((inst >> 0x10) & 0x1F) - 1].w[0];
                } else {
                    wordBuf = cpu->fpr[(inst >> 0x10) & 0x1F].w[1];
                }

                dev = get_dev(devs, dev_idx, ea);
                dev->sw(dev->dev_obj, ea + dev->addr_offset, (s32*)&wordBuf);
            }
            break;
        case 60: // SCD
            {
                tmp.d = cpu->gpr[MIPS_RT(inst)].d; 
                ea = cpu->gpr[MIPS_RS(inst)].sd + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);
                
                cpu->gpr[(inst >> 0x10) & 0x1F].sd = !!dev->sd(dev->dev_obj, ea + dev->addr_offset, &tmp.sd);
            }
            break;
        case 61: // SDC1
            {
                tmp.d = cpu->fpr[(inst >> 0x10) & 0x1F].d;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                dev->sd(dev->dev_obj, ea + dev->addr_offset, (s64*)&tmp.d);
            }
            break;
        case OPC_SD:
            {
                tmp.d = cpu->gpr[(inst >> 0x10) & 0x1F].d;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = get_dev(devs, dev_idx, ea);

                dev->sd(dev->dev_obj, ea + dev->addr_offset, (s64*)&tmp.d);
            }
            break;

    }

    if(!cpuExecuteUpdate(cpu, &ret, tick + 1)) {
        return NULL;
    }

    if(save_ra) {
        cpu->gpr[0x1F].d = prev_ra;
    }

    cpu->unk_0x24 = -1;
    cpu->tick = OSGetTick();

    return ret;
}
#else
void *cpuExecuteOpcode(cpu_class_t *cpu, s32 arg1, u32 addr, u32 *ret);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteOpcode.s")
#endif
#undef NON_MATCHING

#ifdef NON_MATCHING
void *cpuExecuteIdle(cpu_class_t *cpu, u32 arg1, u32 addr, u32 *ret) {
    rom_class_t *rom = gSystem->rom;
    u32 tick = OSGetTick();

    if(cpu->unk_0x24 != 0) {
        cpu->status |= 8;
    } else {
        cpu->status &= ~8;
    }

    cpu->pc = addr;
    cpu->status |= 0x80;

    if(!(cpu->status & 0x40) && !rom->unk_19A34) {
        videoForceRetrace(gSystem->video);
    }

    if(!cpuExecuteUpdate(cpu, &ret, tick)) {
        return NULL;
    }

    cpu->tick = OSGetTick();
    
    return ret;
}
#else
void *cpuExecuteIdle(cpu_class_t *cpu, u32 r4, u32 addr, u32 *ret);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteIdle.s")
#endif

void *cpuExecuteJump(cpu_class_t *cpu, u32 r4, u32 addr, u32 *ret) {
    u32 tick = OSGetTick();

    if(cpu->unk_0x24 != 0) {
        cpu->status |= 8;
    } else {
        cpu->status &= ~8;
    }

    cpu->pc = addr;
    cpu->status |= 4;
    if(!cpuExecuteUpdate(cpu, &ret, tick)) {
        return NULL;
    }

    cpu->tick = OSGetTick();

    return ret;
}

inline void findExternalCall(cpu_class_t *cpu, s32 addr, u32 *ret) {
    s32 i;
    recomp_node_t *node;
    ext_call_t *ext_call;
    u32 *ret_p = ret - 1;
    
    cpu->call_cnt++;
    cpuFindFunction(cpu, cpu->n64_ra - 8, &node);

    ext_call = node->ext_calls;
    for(i = 0; i < node->ext_call_cnt; ext_call++, i++) {
        if(addr == ext_call->n64_addr) {
            if(ext_call->vc_addr == NULL) {
                node->ext_calls[i].vc_addr = ret_p;
                return;
            }
        }
    }
}

/**
 * @brief Executes a call from the dyanrec environment
 * 
 * @param cpu The emulated VR4300.
 * @param arg1 
 * @param addr Address of the call.
 * @param ret Return address after the call has completed.
 * @return void* A pointer to the recompiled called function.
 */
void *cpuExecuteCall(cpu_class_t *cpu, u32 arg1, s32 addr, u32 *ret) {
    recomp_node_t *node;
    s32 i;
    s32 flg;
    u32 offset;
    u32 *ret_p;
    u32 tick = OSGetTick();

    if(cpu->unk_0x24 != 0) {
        cpu->status |= 8;
    } else {
        cpu->status &= ~8;
    }

    cpu->status |= 4;
    cpu->pc = addr;
    cpu->gpr[MREG_RA].w[1] = (u32)ret;

    findExternalCall(cpu, addr, ret);

    flg = (reg_map[MREG_RA] & 0x100) != 0;
    ret_p = ret - (!!flg + 3);
    if(flg) {
        /**
         * lis r5, hi(ret)
         * ori r5, r5, lo(ret)
         */
        ret_p[0] = (15 << 26) | ((u32)ret >> 0x10) | (5 << 0x15) | (0 << 0x10);
        ret_p[1] = (24 << 26) | ((u32)ret & 0xFFFF) |(5 << 0x15) | (5 << 0x10);
        DCStoreRange(ret_p, 8);
        ICInvalidateRange(ret_p, 8);
    } else {
        u32 reg = reg_map[MREG_RA];
        /**
         * lis rA, hi(ret)
         * ori rA, rA, lo(ret)
         */
        ret_p[0] = (15 << 26) | ((u32)ret >> 0x10) | (reg << 0x15) | (0 << 0x10);
        ret_p[1] = (24 << 26) | ((u32)ret & 0xFFFF) | (reg << 0x15) | (reg << 0x10);  
        DCStoreRange(ret_p, 8);
        ICInvalidateRange(ret_p, 8);
    }

    if(!cpuExecuteUpdate(cpu, &ret, tick)) {
        return NULL;
    }

    offset = (u32)ret - (u32)&ret_p[3];
    if(flg) {
        /**
         * b ret
         */
        ret_p[3] = (offset & 0x3FFFFFC) | 0x48000000;
        DCStoreRange(ret_p, 16);
        ICInvalidateRange(ret_p, 16);
    } else {
        /**
         * b ret
         */
        ret_p[2] = (offset & 0x3FFFFFC) | 0x48000000;
        DCStoreRange(ret_p, 12);
        ICInvalidateRange(ret_p, 12);
    }

    cpu->tick = OSGetTick();

    return ret;
}


#ifdef NON_MATCHING
// very minor regalloc
/**
 * @brief Recompiles a VR4300 load/store instruction
 * 
 * @param cpu The emulated VR4300.
 * @param arg1 
 * @param addr The address of the Load/Store instruction.
 * @param ret A pointer to the location where recompiled code should be stored.
 * @return void* The location where the dynarec block should continue executing.
 */
void *cpuExecuteLoadStore(cpu_class_t *cpu, u32 arg1, u32 addr, u32 *ret) {
    s32 pos = 0;
    s32 tmp3 = 0;
    s32 tmp1 = 0;
    u32 *buf;
    u32 rt;
    u32 base;
    s32 i;
    u8 dev_idx;
    u32 *tmp2;
    u32 *puVar2;
    s32 inst_search = 0x90C30000 + ((u32)&cpu->unk_0x24 - (u32)cpu);
    u32 *ret_p;
    u32 ea;

    if(!cpuGetAddressBuffer(cpu, (void**)&buf, addr)) {
        return 0;
    }

    ea = cpu->gpr[MIPS_RS(*buf)].sw[1] + MIPS_IMM(*buf);
    dev_idx = cpu->mem_hi_map[ea >> 16];
    if(cpu->unk_0x12224 & 0x100) {
        ret_p = &ret[-3];
        puVar2 = ret + 3;
        tmp2 =  &ret_p[-2];
    } else {
        ret_p = &ret[-3];
        puVar2 = ret + 2;
        tmp2 = &ret_p[-2];  
    }

    if((ea >> 0x1C) < 8) {
        tmp1 = 1;
    }

    if(!tmp1 && dev_idx >= 128) {
        switch(MIPS_OP(*buf)) {
            case OPC_LB:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 5;
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;

                    ret_p[pos++] = 0x80c30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) {
                        ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x88070000 | (rt << 21) | (*buf & 0xFFFF);
                ret_p[pos++] = 0x7C000774 | (rt << 21) | (rt << 16);

                if(reg_map[MIPS_RT(*buf)] & 0x100) {
                    ret_p[pos++] = 0x90A30000 + (u16)((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                break;
            case OPC_LBU:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 5;
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;

                    ret_p[pos++] = 0x80c30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x88070000 | (rt << 21) | (*buf & 0xFFFF);

                if(reg_map[MIPS_RT(*buf)] & 0x100) {
                    ret_p[pos++] = 0x90A30000 + (u16)((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                break;
            case OPC_LH:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 5;
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;

                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] -(u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0xA0070000 | (rt << 21) | (*buf & 0xFFFF);
                ret_p[pos++] = 0x7C000734 | (rt << 21) | (rt << 16);

                if(reg_map[MIPS_RT(*buf)] & 0x100) {
                    ret_p[pos++] = 0x90A30000 + (u16)((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                break;

            case OPC_LHU:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 5;
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;

                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] -(u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0xA0070000 | (rt << 21) | (*buf & 0xFFFF);

                if(reg_map[MIPS_RT(*buf)] & 0x100) {
                    ret_p[pos++] = 0x90A30000 + (u16)((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                break;
            case OPC_LW:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 5;
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;

                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x80070000 | (rt << 21) | (*buf & 0xFFFF);

                if(reg_map[MIPS_RT(*buf)] & 0x100) {
                    ret_p[pos++] = 0x90A30000 + (u16)((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                break;
            case OPC_SB:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 6;
                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 7;
                    ret_p[pos++] = 0x80E30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) { 
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x98070000 | (rt << 21) | (*buf & 0xFFFF);
                break;
            case OPC_SH:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 6;
                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 7;
                    ret_p[pos++] = 0x80E30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) { 
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0xB0070000 | (rt << 21) | (*buf & 0xFFFF);
                break;
            case OPC_SW:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 6;
                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 7;
                    ret_p[pos++] = 0x80E30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) { 
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x90070000 | (rt << 21) | (*buf & 0xFFFF);
                break;
            default:
                invalidInst();
                break;
        }   
    } else {
        // regalloc is here, the or of 0x3CA00000 and hi bits of addr are placed in r4 instead of r3
        tmp1 = 1;
        ret_p[pos++] = 0x3CA00000 | (addr >> 0x10);
        ret_p[pos++] = 0x60A50000 | (addr & 0xFFFF);
        ret_p[pos++] = 0x48000001 | (((u32)cpu->execute_opcode - (u32)&ret_p[pos]) & 0x3FFFFFC);
    }

    if(cpu->unk_0x12224 & 0x100) {
        if((6 - pos) >= 2) {
            tmp3 = pos;
            ret_p[pos++] = 0x48000000 | (u16)((u32)&ret_p[6] - (u32)&ret_p[pos]);
        }

        while(pos <= 5) {
            ret_p[pos++] = 0x60000000;
        }

        pos = 6;
    } else {
        if((5 - pos) >= 2) {
            tmp3 = pos;
            ret_p[pos++] = 0x48000000 | (u16)((u32)&ret_p[5] - (u32)&ret_p[pos]);
        }

        while(pos <= 4) {
            ret_p[pos++] = 0x60000000;
        }

        pos = 5;
    }

    if(!tmp1 && tmp2[0] == 0x38C00000 && (s32)tmp2[1] == inst_search) {
        s32 pos2 = 0;
        tmp2[pos2++] = 0x48000000 | (u16)((u32)&tmp2[2] - (u32)tmp2);
        tmp2[pos2++] = 0x60000000;
        DCStoreRange(tmp2, pos2 * sizeof(*tmp2));
        ICInvalidateRange(tmp2, pos2 * sizeof(*tmp2));
        if(tmp3 != 0) {
            ret_p[tmp3] = 0x48000000 | (u16)((u32)&puVar2[2] - (u32)&ret_p[tmp3]);
        }

        puVar2[0] = 0x60000000;
        puVar2[1] = 0x60000000;
        cpu->unk_0x24 = -1;
        pos += 2;
    }

    DCStoreRange(ret_p, pos * 4);
    ICInvalidateRange(ret_p, pos * 4);

    return ret_p;
}
#else
void *cpuExecuteLoadStore(cpu_class_t *cpu, u32 arg1, u32 addr, u32 *ret);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteLoadStore.s")
#endif
#undef NON_MATCHING


#ifdef NON_MATCHING
/**
 * @brief Recompiles a VR4300 load/store instruction on COP1 or doubleword load/store.
 * 
 * @param cpu The emulated VR4300.
 * @param arg1 
 * @param addr The address of the Load/Store instruction.
 * @param ret A pointer to the location where recompiled code should be stored.
 * @return void* The location where the dynarec block should continue executing.
 */
void *cpuExecuteLoadStoreF(cpu_class_t *cpu, u32 arg1, u32 addr, u32 *ret) {
    u32 tmp3;
    u32 tmp1;
    s32 inst_search = 0x90C30000 + ((u32)&cpu->unk_0x24 - (u32)cpu);
    u32 *tmp2;
    u32 *puVar2;
    u32 *ret_p;
    s32 pos;
    u32 ea;
    u8 dev_idx;
    u32 *buf;
    u32 rt;
    u32 base;
    s32 ft;

    pos = 0;
    tmp3 = 0;
    tmp1 = 0;
    if(!cpuGetAddressBuffer(cpu, (void**)&buf, addr)) {
        return NULL;
    }

    ea = cpu->gpr[MIPS_RS(*buf)].sw[1] + MIPS_IMM(*buf);
    dev_idx = cpu->mem_hi_map[ea >> 16];
    if(cpu->unk_0x12224 & 0x100) {
        ret_p = &ret[-3];
        puVar2 = ret + 4;
        tmp2 = &ret_p[-2];
    } else {
        ret_p = &ret[-3];
        puVar2 = ret + 3;
        tmp2 = &ret_p[-2];
    }

    if((ea >> 0x1C) < 8) {
        tmp1 = 1;
    }

    if(!tmp1 && dev_idx >= 128) {
        ft = MIPS_FT(*buf);
        switch(MIPS_OP(*buf)) {
            case OPC_LWC1:
                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;
                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100 && ((ea >> 0x1C) >= 10)) {
                    ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);

                if((ft % 2) == 1) {
                    ret_p[pos++] = 0x80A70000 | (*buf & 0xFFFF);
                    ret_p[pos++] = 0x90A30000 + ((u32)&cpu->fpr[ft - 1] - (u32)cpu);
                } else {
                    ret_p[pos++] = 0x80A70000 | (*buf & 0xFFFF);
                    ret_p[pos++] = 0x90A30000 + ((u32)&cpu->fpr[ft] - (u32)cpu + 4);
                }
                break;
            case OPC_SWC1:
                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;
                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100 && ((ea >> 0x1C) >= 10)) {
                    ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                if((ft % 2) == 1){  
                    ret_p[pos++] = 0x80A30000 + ((u32)&cpu->fpr[ft - 1] - (u32)cpu);
                } else {
                    ret_p[pos++] = 0x80A30000 + ((u32)&cpu->fpr[ft] - (u32)cpu + 4);
                }

                ret_p[pos++] = 0x90A70000 | (*buf & 0xFFFF);
                break;
            case OPC_LDC1:
                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;
                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100 && ((ea >> 0x1C) >= 10)) {
                    ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x80A70000 | (*buf & 0xFFFF);
                ret_p[pos++] = 0x90A30000 + ((u32)&cpu->fpr[ft] - (u32)cpu);
                ret_p[pos++] = 0x80A70000 | ((*buf & 0xFFFF) + 4);
                ret_p[pos++] = 0x90A30000 + ((u32)&cpu->fpr[ft] - (u32)cpu + 4);
                break;
            case OPC_SDC1:
                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;
                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100 && ((ea >> 0x1C) >= 10)) {
                    ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x80A30000 + ((u32)&cpu->fpr[ft] - (u32)cpu);
                ret_p[pos++] = 0x90A70000 | (*buf & 0xFFFF);
                ret_p[pos++] = 0x80A30000 + ((u32)&cpu->fpr[ft] - (u32)cpu + 4);
                ret_p[pos++] = 0x90A70000 | ((*buf & 0xFFFF) + 4);
                break;
            case OPC_LD:
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 5;
                }

                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 6;
                    ret_p[pos++] = 0x80C30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) {
                        ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x80A70000 | (*buf & 0xFFFF);
                ret_p[pos++] = 0x90A30000  + ((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu);
                ret_p[pos++] = 0x80070000 | (rt << 21) | ((*buf & 0xFFFF) + 4);
                ret_p[pos++] = (0x90030000 | (rt << 21)) + ((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                break;
            case OPC_SD:
                base = reg_map[MIPS_RS(*buf)];
                if(base & 0x100) {
                    base = 7;
                    ret_p[pos++] = 0x80E30000 + (u16)((u32)&cpu->gpr[MIPS_RS(*buf)] - (u32)cpu + 4);
                }

                if(cpu->unk_0x12224 & 0x100) {
                    if(cpu->unk_0x12224 & 0x1000) {
                        ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                    } else {
                        if((ea >> 0x1C) >= 10) {
                            ret_p[pos++] = 0x7C000038 | (base << 21) | (base << 16) | 0x4800;
                        }
                    }
                }

                ret_p[pos++] = 0x7CE04214 | (base << 16);
                ret_p[pos++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu);
                ret_p[pos++] = 0x90C70000 | (*buf & 0xFFFF);
                rt = reg_map[MIPS_RT(*buf)];
                if(rt & 0x100) {
                    rt = 6;
                    ret_p[pos++] = 0x80C30000 + ((u32)&cpu->gpr[MIPS_RT(*buf)] - (u32)cpu + 4);
                }

                ret_p[pos++] = 0x90070000 | (rt << 21) | ((*buf & 0xFFFF) + 4);
                break;
            default:
                invalidInst();
                break;
        }
    } else {
        tmp1 = 1;
        ret_p[pos++] = 0x3CA00000 | (addr >> 0x10);
        ret_p[pos++] = 0x60A50000 | (addr & 0xFFFF);
        ret_p[pos++] = 0x48000001 | (((u32)cpu->execute_opcode - (u32)&ret_p[pos]) & 0x3FFFFFC);
    }

    if(cpu->unk_0x12224 & 0x100) {
        if((7 - pos) >= 2) {
            tmp3 = pos;
            ret_p[pos++] = 0x48000000 | (u16)((u32)&ret_p[7] - (u32)&ret_p[pos]);
        }

        while(pos <= 6) {
            ret_p[pos++] = 0x60000000;
        }

        pos = 7;
    } else {
        if((6 - pos) >= 2) {
            tmp3 = pos;
            ret_p[pos++] = 0x48000000 | (u16)((u32)&ret_p[6] - (u32)&ret_p[pos]);
        }

        while(pos <= 5) {
            ret_p[pos++] = 0x60000000;
        }

        pos = 6;
    }

    if(!tmp1 && tmp2[0] == 0x38C00000 && (s32)tmp2[1] == inst_search) {
        s32 pos2 = 0;
        tmp2[pos2++] = 0x48000000 | (u16)((u32)&tmp2[2] - (u32)tmp2);
        tmp2[pos2++] = 0x60000000;
        DCStoreRange(tmp2, pos2 * sizeof(*tmp2));
        ICInvalidateRange(tmp2, pos2 * sizeof(*tmp2));
        if(tmp3 != 0) {
            ret_p[tmp3] = 0x48000000 | (u16)((u32)&puVar2[2] - (u32)&ret_p[tmp3]);
        }

        puVar2[0] = 0x60000000;
        puVar2[1] = 0x60000000;
        cpu->unk_0x24 = -1;
        pos += 2;
    }

    DCStoreRange(ret_p, pos * sizeof(*ret_p));
    ICInvalidateRange(ret_p, pos * sizeof(*ret_p));

    return ret_p;
}
#else
void *cpuExecuteLoadStoreF(cpu_class_t *cpu, u32 arg1, u32 addr, u32 *ret);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteLoadStoreF.s")
#endif
#undef NON_MATCHING

class_t gClassCPU = {
    cpu_class_name,
    sizeof(cpu_class_t),
    0,
    cpuEvent
};

/**
 * @brief Generates a call to a virtual-console function from within the dynarec envrionment
 * Dedicated PPC registers are saved to the cpu object, and restored once the virtual-console function has finished.
 * Jump to the return value of the virtual-console function
 * @param cpu The emulated VR4300.
 * @param link_code A pointer to store the gneerated PPC code.
 * @param handler The virtual-console function to call.
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuMakeLink(cpu_class_t *cpu, u32 **link_code, void *handler) {
    u32 *code;
    s32 i;
    u32 paddr;
    s32 cnt = sizeof(reg_map) / sizeof(*reg_map);
    
    if(!xlHeapTake((void**)&code, 0x30000200)) {
        return 0;
    }

    *link_code = code;

    // mflr r6
    *code++ = 0x7CC802A6;

    for(i = 1; i < 32; i++) {
        if(!(reg_map[i] & 0x100)) {
            // stw rx, off(r3)
            *code++ = (((u32)&cpu->gpr[i] - (u32)cpu) + 4) | (reg_map[i] << 0x15) | 0x90030000;
        }
    }

    /**
     * bl handler
     * mtlr r3
     * lis r3, hi(cpu)
     * ori r3, lo(cpu)
     * lwz r4, cpu->call_cntz
     */
    *code++ = (((u32)handler - (u32)code) & 0x3FFFFFC) | 0x48000001;
    *code++ = 0x7C6803A6;
    *code++ = ((u32)cpu >> 0x10) | 0x3C600000;
    *code++ = ((u32)cpu & 0xFFFF) | 0x60630000;
    *code++ = (((s32)&cpu->call_cnt - (s32)cpu)) + 0x80830000;

    paddr = (u32)gSystem->ram->dram - 0x80000000;

    // lui r8, hi(dram)
    *code++ = (paddr >> 0x10) | 0x3D000000;
    if(cpu->unk_0x12224 & 0x100) {
        /**
         * lis r9, 0xDFFF
         * ori r8, r8, lo(dram)
         * ori r9, r9, 0xFFFF
         */
        *code++ = 0x3D20DFFF;
        *code++ = (paddr & 0xFFFF) | 0x61080000;
        *code++ = 0x6129FFFF;
    } else if(cpu->unk_0x12224 & 1) {
        /**
         * addi r9, r3, 0xF60
         * ori r8, r8, lo(dram)
         */
        *code++ = (((u32)cpu->mem_hi_map - (u32)cpu)) + 0x39230000;                                   
        *code++ = (paddr & 0xFFFF) | 0x61080000;
    }


    // li r10, 0
    *code++ = (reg_map[0] << 0x15) | 0x38000000;

    for(i = 1; i < 32; i++) {
        if(!(reg_map[i] & 0x100)) {
            // lwz rx, off(r3)
            *code++ = (((u32)&cpu->gpr[i] - (u32)cpu) + 4) | (reg_map[i] << 0x15) | 0x80030000;
        }
    }

    // blr
    *code++ = 0x4E800020;

    DCStoreRange(*link_code, 0x200);
    ICInvalidateRange(*link_code, 0x200);

    return 1;
}

#ifdef NON_MATCHING
extern void (*lbl_8025CFE8)(u32);

inline s32 __free(void **ptr) {
    void ***tmp = &ptr;
    if(!xlHeapFree((void**)tmp)) {
        return 0;
    }

    **tmp = NULL;
    return 1;
}

inline s32 cpuCompile_DADD(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x3000000C)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0x7CA53814; // addc r5, r5, r7
    buf[1] = 0x7CC64114; // adde r6, r6, r8
    buf[2] = 0x4E800020; // blr
    DCStoreRange(buf, 12);
    ICInvalidateRange(buf, 12);

    return 1;
}

inline s32 cpuCompile_DADDU(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x3000000C)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0x7CA53814; // addc r5, r5, r7
    buf[1] = 0x7CC64114; // adde r6, r6, r8
    buf[2] = 0x4E800020; // blr
    DCStoreRange(buf, 12);
    ICInvalidateRange(buf, 12);

    return 1;
}

inline s32 cpuCompile_DSUB(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x3000000C)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0x7CA53814; // addc r5, r5, r7
    buf[1] = 0x7CC64114; // adde r6, r6, r8
    buf[2] = 0x4E800020; // blr
    DCStoreRange(buf, 12);
    ICInvalidateRange(buf, 12);

    return 1;
}

inline s32 cpuCompile_DSUBU(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x3000000C)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0x7CA53814; // addc r5, r5, r7
    buf[1] = 0x7CC64114; // adde r6, r6, r8
    buf[2] = 0x4E800020; // blr
    DCStoreRange(buf, 12);
    ICInvalidateRange(buf, 12);

    return 1;
}

inline s32 cpuCompile_ROUND_W(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x3000000C)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0xFC00081C; // addc r5, r5, r7
    buf[1] = 0x7C051FAE; // adde r6, r6, r8
    buf[2] = 0x4E800020; // blr
    DCStoreRange(buf, 12);
    ICInvalidateRange(buf, 12);

    return 1;
}

inline s32 cpuCompile_TRUNC_W(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x3000000C)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0xFC00081C; // addc r5, r5, r7
    buf[1] = 0x7C051FAE; // adde r6, r6, r8
    buf[2] = 0x4E800020; // blr
    DCStoreRange(buf, 12);
    ICInvalidateRange(buf, 12);

    return 1;
}

/**
 * @brief Begins execution of the emulated VR4300
 * 
 * @param cpu The emulated VR4300
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuExecute(cpu_class_t *cpu) {
    u32 *dadd_buf;
    u32 *daddu_buf;
    u32 *dsub_buf;
    u32 *dsubu_buf;
    u32 *round_w_buf;
    u32 *trunc_w_buf;
    u32 *entry_buf;
    u32 *entry_p;
    s32 entry_pos;
    recomp_node_t *entry_node;
    u32 ram;
    s32 i;

    if(cpu->unk_0x12224 & 0x1000) {
        cpu->unk_0x12224 |= 0x100;
    }

    if(!cpuMakeLink(cpu, (u32**)&cpu->execute_opcode, cpuExecuteOpcode)) {
        return 0;
    }

    if(!cpuMakeLink(cpu, (u32**)&cpu->execute_jump, cpuExecuteJump)) {
        return 0;
    }

    if(!cpuMakeLink(cpu, (u32**)&cpu->execute_call, cpuExecuteCall)) {
        return 0;
    }

    if(!cpuMakeLink(cpu, (u32**)&cpu->execute_idle, cpuExecuteIdle)) {
        return 0;
    }

    if(!cpuMakeLink(cpu, (u32**)&cpu->execute_loadstore, cpuExecuteLoadStore)) {
        return 0;
    }

    if(!cpuMakeLink(cpu, (u32**)&cpu->execute_loadstoref, cpuExecuteLoadStoreF)) {
        return 0;
    }

    cpuCompile_DSLLV(cpu, &cpuCompile_DSLLV_function);
    cpuCompile_DSRLV(cpu, &cpuCompile_DSRLV_function);
    cpuCompile_DSRAV(cpu, &cpuCompile_DSRAV_function);
    cpuCompile_DMULT(cpu, &cpuCompile_DMULT_function);
    cpuCompile_DMULTU(cpu, &cpuCompile_DMULTU_function);
    cpuCompile_DDIV(cpu, &cpuCompile_DDIV_function);
    cpuCompile_DDIVU(cpu, &cpuCompile_DDIVU_function);

    // inlined
    cpuCompile_DADD(cpu, &cpuCompile_DADD_function);
    cpuCompile_DADDU(cpu, &cpuCompile_DADDU_function);
    cpuCompile_DADDU(cpu, &cpuCompile_DSUB_function);
    cpuCompile_DADDU(cpu, &cpuCompile_DSUBU_function);

    cpuCompile_S_SQRT(cpu, &cpuCompile_S_SQRT_function);
    cpuCompile_D_SQRT(cpu, &cpuCompile_D_SQRT_function);
    cpuCompile_W_CVT_SD(cpu, &cpuCompile_W_CVT_SD_function);
    cpuCompile_L_CVT_SD(cpu, &cpuCompile_L_CVT_SD_function);
    cpuCompile_CEIL_W(cpu, &cpuCompile_CEIL_W_function);
    cpuCompile_FLOOR_W(cpu, &cpuCompile_FLOOR_W_function);

    // inlined
    cpuCompile_ROUND_W(cpu, &cpuCompile_ROUND_W_function);
    cpuCompile_TRUNC_W(cpu, &cpuCompile_TRUNC_W_function);

    cpuCompile_LB(cpu, &cpuCompile_LB_function);
    cpuCompile_LH(cpu, &cpuCompile_LH_function);
    cpuCompile_LW(cpu, &cpuCompile_LW_function);
    cpuCompile_LBU(cpu, &cpuCompile_LBU_function);
    cpuCompile_LHU(cpu, &cpuCompile_LHU_function);
    cpuCompile_SB(cpu, &cpuCompile_SB_function);
    cpuCompile_SH(cpu, &cpuCompile_SH_function);
    cpuCompile_SW(cpu, &cpuCompile_SW_function);
    cpuCompile_LDC(cpu, &cpuCompile_LDC_function);
    cpuCompile_SDC(cpu, &cpuCompile_SDC_function);
    cpuCompile_LWL(cpu, &cpuCompile_LWL_function);
    cpuCompile_LWR(cpu, &cpuCompile_LWR_function);

    if(cpuMakeFunction(cpu, &entry_node, cpu->pc)) {
        if(!xlHeapTake((void**)&entry_buf, 0x30000100)) {
            return 0;
        }

        entry_pos = 0;
        entry_p = entry_buf;
        *entry_p++ = 0x3c600000 | ((u32)cpu >> 16);
        *entry_p++ = 0x60630000 | ((u32)cpu & 0xFFFFF);
        *entry_p++ = 0x80830000 | ((u32)&cpu->call_cnt - (u32)cpu);
        ram = (u32)gSystem->ram->dram - 0x80000000;
        *entry_p++ = 0x3d000000 | (ram >> 16);
        *entry_p++ = 0x61080000 | (ram & 0xFFFF);
        if(cpu->unk_0x12224 & 0x100) {
            *entry_p++ = 0x3D20DFFF;
            *entry_p++ = 0x6129FFFF;
        } else {
            if(cpu->unk_0x12224 & 1) {
                *entry_p++ = 0x39230000 | ((u32)cpu->mem_hi_map - (u32)cpu);
            }
        }

        for(i = 0; i < sizeof(reg_map) / sizeof(*reg_map); i++) {
            if(!(reg_map[i] & 0x100)) {
                *entry_p++ = 0x80030000 | ((u32)&cpu->gpr[i] - (u32)cpu + 4) | (reg_map[i] << 0x15);
            }
        }
        *entry_p++ = 0x48000000 | (((u32)entry_node->recompiled_func - (u32)entry_p) & 0x3FFFFFC);
        DCStoreRange(entry_buf, 0x100);
        ICInvalidateRange(entry_buf, 0x100);
        cpu->unk_0x40 = 0;
        cpu->unk_0x3C = 0;
        VIWaitForRetrace();
        lbl_8025CFE8 = VISetPostRetraceCallback(func_80032B74);
        ((void(*)())entry_buf)(); // Does not return normally.
        if(!xlHeapFree((void**)&entry_buf)) {
            return 0;
        }

        if(!__free((void**)&cpu->execute_idle)) {
            return 0;
        }

        if(!__free((void**)&cpu->execute_call)) {
            return 0;
        }

        if(!__free((void**)&cpu->execute_jump)) {
            return 0;
        }

        if(!__free((void**)&cpu->execute_opcode)) {
            return 0;
        }

        if(!__free((void**)&cpu->execute_loadstore)) {
            return 0;
        }

        if(!__free((void**)&cpu->execute_loadstoref)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DSLLV_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DSRLV_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DSRAV_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DMULT_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DMULTU_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DDIV_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DDIVU_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DADD_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DADDU_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DSUB_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_DSUBU_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_S_SQRT_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_D_SQRT_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_W_CVT_SD_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_L_CVT_SD_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_CEIL_W_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_FLOOR_W_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_TRUNC_W_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_ROUND_W_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_LB_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_LH_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_LW_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_LBU_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_LHU_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_SB_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_SH_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_SW_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_LDC_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_SDC_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_LWL_function)) {
            return 0;
        }

        if(!xlHeapFree((void**)&cpuCompile_LWR_function)) {
            return 0;
        }
    }

    return 1;
}
#else
s32 cpuExecute(cpu_class_t *cpu);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecute.s")
#endif
#undef NON_MATCHING

/**
 * @brief Maps an object to a cpu device.
 * 
 * @param cpu The emulated VR4300.
 * @param dev The device that will handle requests for this memory space.
 * @param address_start The start of the memory space for which the device will be responsible.
 * @param address_end The end of the memory space for which the device will be responsible.
 * @param create_arg An argument which will be passed back to the device on creation.
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuMapObject(cpu_class_t *cpu, void *dev, u32 address_start, u32 address_end, u32 create_arg){
    u32 size = address_end - address_start + 1;
    u32 dev_idx;

    if(address_start == 0 && address_end == 0xFFFFFFFF) {
        if(!cpuMakeDevice(cpu, &dev_idx, dev, 0, 0, size, create_arg)) {
            return 0;
        }

        cpu->mem_dev_idx = dev_idx;
    } else {
        if(!cpuMakeDevice(cpu, &dev_idx, dev, address_start | 0x80000000, address_start, size, create_arg)) {
            return 0;
        }

        if(!cpuMakeDevice(cpu, &dev_idx, dev, address_start | 0xA0000000, address_start, size, create_arg)) {
            return 0;
        }
    }

    return 1;
}

s32 cpuGetBlock(cpu_class_t *cpu, cpu_blk_req_t *req) {
    u32 dev_addr;
    cpu_dev_t *dev;
    s32 i;
    
    dev_addr = req->dev_addr;
    if(dev_addr < 0x4000000) {
        dev_addr = req->dst_phys_ram;
    }
    
    i = 1;
    while(cpu->devices[i] != NULL) {
        dev = cpu->devices[i];
        if(dev->paddr_start <= dev_addr && dev_addr <= dev->paddr_end) {
            if(dev->get_blk != NULL) {
                return dev->get_blk(dev->dev_obj, req);
            }

            return 0;
        }
        i++;
    }

    dev = cpu->devices[cpu->mem_dev_idx];
    if(dev != NULL && dev->get_blk != NULL) {
        return dev->get_blk(dev->dev_obj, req);
    }

    return 0;
}

s32 cpuSetGetBlock(cpu_class_t* cpu, cpu_dev_t* dev, void* get_blk) {
    dev->get_blk = get_blk;

    return 1;
}

/**
 * @brief Sets load handlers for a device.
 * 
 * @param cpu The emulated VR4300.
 * @param dev The device which handles the load operations.
 * @param lb byte handler.
 * @param lh halfword handler.
 * @param lw word handler.
 * @param ld doubleword handler.
 * @return s32 1 on success, 0 otherwise.
 */
s32 cpuSetDeviceGet(cpu_class_t* cpu, cpu_dev_t* dev, lb_func_t lb, lh_func_t lh, lw_func_t lw, ld_func_t ld)  {
    dev->lb = lb;
    dev->lh = lh;
    dev->lw = lw;
    dev->ld = ld;

    return 1;
}

/**
 * @brief Sets store handlers for a device.
 * 
 * @param cpu The emulated VR4300.
 * @param dev The device which handles the store operations.
 * @param lb byte handler.
 * @param lh halfword handler.
 * @param lw word handler.
 * @param ld doubleword handler.
 * @return s32 1 on success, 0 otherwise.
*/
s32 cpuSetDevicePut(cpu_class_t* cpu, cpu_dev_t* dev, sb_func_t sb, sh_func_t sh, sw_func_t sw, sd_func_t sd)  {
    dev->sb = sb;
    dev->sh = sh;
    dev->sw = sw;
    dev->sd = sd;

    return 1;
}

s32 cpuSetCodeHack(cpu_class_t *cpu, u32 addr, u32 repl, s32 end) {
    s32 i;

    for(i = 0; i < cpu->hack_cnt; i++) {
        if(addr == cpu->hacks[i].addr) {
            return 0;
        }
    }

    cpu->hacks[i].addr = addr;
    cpu->hacks[i].expected = repl;
    cpu->hacks[i].replacement = end;
    cpu->hack_cnt++;

    return 1;
}

#ifdef NON_MATCHING
// Matches except for  double constant.
inline s32 clearblk(u32 *blk, s32 cnt) {
    s32 i;
    for(i = 0; i < cnt; i++) {
        blk[i] = 0;
    }
}

s32 cpuReset(cpu_class_t *cpu) {
    s32 i;
    cpu->unk_0x04 = 0;
    cpu->hack_cnt = 0;
    cpu->status = 0x40;
    cpu->execute_opcode = NULL;

    for(i = 0; i < 48; i += 4) {
        // Maybe fake?  Could be manual un rolling
        cpu->tlb[i + 0].entry_lo0.d = 0;
        cpu->tlb[i + 0].entry_lo1.d = 0;
        cpu->tlb[i + 0].entry_hi.d = 0;
        cpu->tlb[i + 0].page_mask.d = 0;
        cpu->tlb[i + 0].dev_status.d = -1;

        cpu->tlb[i + 1].entry_lo0.d = 0;
        cpu->tlb[i + 1].entry_lo1.d = 0;
        cpu->tlb[i + 1].entry_hi.d = 0;
        cpu->tlb[i + 1].page_mask.d = 0;
        cpu->tlb[i + 1].dev_status.d = -1;

        cpu->tlb[i + 2].entry_lo0.d = 0;
        cpu->tlb[i + 2].entry_lo1.d = 0;
        cpu->tlb[i + 2].entry_hi.d = 0;
        cpu->tlb[i + 2].page_mask.d = 0;
        cpu->tlb[i + 2].dev_status.d = -1;
        
        cpu->tlb[i + 3].entry_lo0.d = 0;
        cpu->tlb[i + 3].entry_lo1.d = 0;
        cpu->tlb[i + 3].entry_hi.d = 0;
        cpu->tlb[i + 3].page_mask.d = 0;
        cpu->tlb[i + 3].dev_status.d = -1;
    }

    cpu->lo.d = 0;
    cpu->hi.d = 0;
    cpu->pc = 0x80000400;
    cpu->unk_0x24 = -1;

    for(i = 0; i < 32; i++) {
        cpu->gpr[i].d = 0;
    }

    for(i = 0; i < 32; i++) {
        cpu->fpr[i].fd = 0.0;
    }

    for(i = 0; i < 32; i++) {
        cpu->fscr[i] = 0;
    }

    cpu->gpr[0x14].d = 1;
    cpu->gpr[0x16].d = 0x3F;
    cpu->gpr[0x1D].d = 0xA4001FF0;
    
    for(i = 0; i < 32; i++) {
        cpu->cp0[i].d = 0;
    }

    cpu->cp0[0xF].d = 0xB00;
    cpu->cp0[0x9].d = 0x10000000;

    cpuSetCP0Status(cpu, 0x20010000,0x2000FF01, 1);

    cpu->cp0[0x10].d = 0x6E463;
    cpu->cache_cnt = 0;

    if(cpuHackHandler(cpu)) {
        cpu->status |= 0x10;
    }


    clearblk(cpu->sm_blk_status, SM_BLK_CNT);

    if(cpu->sm_blk_code == NULL && !xlHeapTake(&cpu->sm_blk_code, 0x30300000)) {
        return 0;
    }

    clearblk(cpu->lg_blk_status, LG_BLK_CNT);

    if(cpu->lg_blk_code == NULL && !xlHeapTake(&cpu->lg_blk_code, 0x30104000)) {
        return 0;
    }

    clearblk(cpu->tree_blk_status, TREE_BLK_CNT);

    if(cpu->recomp_tree != NULL) {
        treeKill(cpu);
    }

    cpu->unk_0x12224 = 1;
    cpu->unk_0x12228 = 0;
    cpu->unk_0x1222C = 0;
    cpu->unk_0x12230 = 0;
    cpu->unk_0x12234 = 0;
    cpu->unk_0x12238 = 0;
    cpu->unk_0x1223C = 0;
    cpu->unk_0x12240 = 0;
    cpu->unk_0x12244 = 0;
    cpu->unk_0x12248 = 0;
    cpu->unk_0x1224C = 0;
    cpu->unk_0x12250 = 0;
    cpu->unk_0x12254 = 0;
    cpu->unk_0x12258 = 0;
    cpu->unk_0x1225C = 0;
    cpu->unk_0x12260 = 0;
    cpu->unk_0x12264 = 0;
    cpu->unk_0x12268 = 0;
    cpu->unk_0x1226C = 0;

    return 1;
}
#else
s32 cpuReset(cpu_class_t *cpu);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuReset.s")
#endif

s32 cpuGetXPC(cpu_class_t *cpu, u64 *pc, u64 *lo, u64 *hi) {
    if(!xlObjectTest(cpu, &gClassCPU)) {
        return 0;
    }

    if(pc != NULL) {
        *pc = cpu->pc;
    }

    if(lo != NULL) {
        *lo = cpu->lo.d;
    }

    if(hi != NULL) {
        *hi = cpu->hi.d;
    }

    return 1;
}

s32 cpuSetXPC(cpu_class_t *cpu, u32 r4, u64 pc, u64 lo, u64 hi) {
    if(!xlObjectTest(cpu, &gClassCPU)) {
        return 0;
    }
    
    cpu->pc = pc;
    cpu->status |= 4;
    cpu->lo.d = lo;
    cpu->hi.d = hi;

    return 1;
}

inline s32 cpuClearDevice(cpu_class_t *cpu, s32 i) {
    s32 ret;
    if(!xlHeapFree((void**)&cpu->devices[i])) {
        ret = 0;
    } else {
        s32 j;
        cpu->devices[i] = NULL;

        for(j = 0; j < 0x10000; j++) {
            if(cpu->mem_hi_map[j] == i) {
                cpu->mem_hi_map[j] = cpu->mem_dev_idx;
            }
        }
        ret = 1;
    }

    return ret;
}

s32 cpuEvent(void* obj, s32 event, void *arg) {
    cpu_class_t *cpu = obj;
    s32 i;

    switch(event) {
        case 1:
            if(!cpuReset(cpu)) {
                return 0;
            }
            break;

        case 2:
            for(i = 0; i < 256; i++) {
                cpu->devices[i] = NULL;
            }

            if(!cpuReset(cpu)) {
                return 0;
            }

            if(!xlHeapTake((void**)&cpu->recomp_tree_nodes, 0x30046500)) { 
                return 0;
            }
            break;
        case 3:
            for(i = 0; i < 256; i++) {
                if(cpu->devices[i] != NULL) {
                    if(!cpuClearDevice(cpu, i)) {
                        return 0;
                    }

                }
            }
            break;
        case 0:
        case 0x1007:
        case 0x1004:
        case 0x1003:
            break;
        default:
            return 0;
    }

    return 1;
}

s32 cpuGetAddressOffset(cpu_class_t *cpu, u32 *buffer, u32 addr) {
    if(addr >= 0x80000000 && addr < 0xC0000000) {
        *buffer = addr & 0x7FFFFF;
    } else if(cpu->devices[cpu->mem_hi_map[addr >> 0x10]]->create_arg & 0x100) {
        *buffer = addr + cpu->devices[cpu->mem_hi_map[addr >> 0x10]]->addr_offset & 0x7FFFFF;
    } else {
        return 0;
    }

    return 1;
}

s32 cpuGetAddressBuffer(cpu_class_t *cpu, void **buffer, u32 addr) {
    cpu_dev_t *dev = cpu->devices[cpu->mem_hi_map[addr >> 0x10]];

    if(dev->dev_obj == gSystem->ram) {
        if(!ramGetBuffer((ram_class_t*)dev->dev_obj, buffer, addr + dev->addr_offset, NULL)) {
            return 0;
        }
    } else if(dev->dev_obj == gSystem->rom) {
        if(!romGetBuffer(dev->dev_obj, buffer, addr + dev->addr_offset, NULL)) {
            return 0;
        }
    } else if(dev->dev_obj == gSystem->rsp) {
        if(!rspGetBuffer(dev->dev_obj, buffer, addr + dev->addr_offset, NULL)) {
            return 0;
        }
    } else {
        return 0;
    }

    return 1;
}

#ifdef NON_MATCHING
// Some minor reordering at the start
s32 cpuGetOffsetAddress(cpu_class_t *cpu, s32 *arg1, s32 *arg2, s32 paddr, s32 len) {
    u32 page_size;
    u32 page_mask;
    s32 i;
    s32 uVar2 = 0;
    u32 tmp2;

    arg1[uVar2++] = paddr | 0x80000000;
    arg1[uVar2++] = paddr | 0xA0000000;
    
    for(i = 0; i < 48; i++) {
        if(cpu->tlb[i].entry_lo0.d & 2) {
            page_mask = cpu->tlb[i].page_mask.d | 0x1FFF;
            switch(page_mask) {
                case 0x1FFF:
                    page_size = 4 * 1024;
                    break;
                case 0x7FFF:
                    page_size = 16 * 1024;
                    break;
                case 0x1FFFF:
                    page_size = 64 * 1024;
                    break;
                case 0x7FFFF:
                    page_size = 256 * 1024;
                    break;
                case 0x1FFFFF:
                    page_size = 1 * 1024 * 1024;
                    break;
                case 0x7FFFFF:
                    page_size = 4 * 1024 * 1024;
                    break;
                case 0x1FFFFFF:
                    page_size = 16 * 1024 * 1024;
                    break;
                default:
                    return 0;
            }
            
            tmp2 = ((u32)((cpu->tlb[i].entry_lo0.d & 0xFFFFFFC0) << 6) + (paddr & page_mask));
            if(tmp2 < (paddr + len - 1) && (tmp2 + page_size - 1) >= paddr) {
                tmp2 = (cpu->tlb[i].entry_hi.d & 0xFFFFE000);
                arg1[uVar2++] = (tmp2 & ~page_mask) | (paddr & page_mask);
            }
        }
    }

    *arg2 = uVar2;
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuGetOffsetAddress.s")
#endif

s32 cpuInvalidateCache(cpu_class_t *cpu, u32 param_2, s32 param_3) {
    if((param_2 & 0xF0000000) == 0xA0000000) {
        return 1;
    }

    if(!cpuFreeCachedAddress(cpu, param_2, param_3)) {
        return 0;
    }

    func_8003E604(cpu, param_2, param_3);
    return 1;
}

s32 cpuGetFunctionChecksum(cpu_class_t *cpu, s32 *cs_out, recomp_node_t *node) {
    u32 *buf;
    s32 i;
    s32 size;
    s32 cs;
    u32 tmp;

    if(node->checksum != 0) {
        *cs_out = node->checksum;
        return 1;
    }

    if(cpuGetAddressBuffer(cpu, (void**)&buf, node->n64_start_addr)) {
        cs = 0;
        size = ((node->n64_end_addr - node->n64_start_addr) >> 2) + 1;
        
        while(size > 0) {
            size--;
            tmp = *buf;
            tmp = tmp >> 0x1A;
            tmp = tmp << ((size % 5) * 6);
            cs += tmp;
            buf++;
        }

        *cs_out = cs;
        node->checksum = cs;

        return 1;
    }

    return 0;
}

#ifdef NON_MATCHING
s32 cpuHeapTake(char **code, cpu_class_t *cpu, recomp_node_t *func, s32 size) {
    s32 smblk_needed = (size + (0x200 - 1)) / 0x200;
    s32 lgblk_needed = (size + (0xA00 - 1)) / 0xA00;
    s32 type_swapped = 0;
    s32 chunk_found = 0;
    s32 blk_cnt;
    u32 *blks;
    s32 blks_needed;
    s32 i;
    s32 i2;
    s32 jstart;
    s32 j;
    u32 mask;

    do {
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
        
        for(i = 0, i2 = 0; i < blk_cnt; blks++, i++, i2 += 32) {

            if(*blks != 0xFFFFFFFF) {
                j = jstart = 33 - blks_needed;
                mask = (1 << blks_needed) - 1;
                do {
                    if(!(*blks & mask)) {
                        chunk_found = 1;
                        *blks |= mask;
                        func->unk_0x34 = (blks_needed << 0x10) | (i2 + (jstart - j));
                        break;
                    }

                    mask <<= 1;
                    j--;
                } while(j != 0);

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

    } while (!type_swapped);

    if(type_swapped) {
        func->alloc_type = -1;
        func->unk_0x34 = 0xFFFFFFFF;
    }

    return 0;
}
#else
s32 cpuHeapTake(char **code, cpu_class_t *cpu, recomp_node_t *func, s32 size);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuHeapTake.s")
#endif

s32 cpuHeapFree(cpu_class_t *cpu, recomp_node_t *func) {
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

s32 cpuTreeTake(recomp_node_t **node, u32 *node_pos, size_t size) {
    u32 *node_status = gSystem->cpu->tree_blk_status;
    u32 i2;
    s32 node_found = 0;
    s32 i;
    s32 j;
    u32 mask;

    for(i = 0, i2 = 0; i < 125; node_status++, i++, i2 += 32) {
        if(*node_status != 0xFFFFFFFF) {
            mask = 1;

            j = 32;
            do {
                if(!(*node_status & mask)) {
                    node_found = 1;
                    *node_status |= mask;
                    *node_pos = 0x10000 | (i2 + (32 - j));
                    break;
                }
                mask <<= 1;
                j--;
            } while(j != 0);
        }

        if(node_found) {
            break;
        }
    }

    if(!node_found) {
        *node_pos = 0xFFFFFFFF;
        return 0;
    }

    *node = &gSystem->cpu->recomp_tree_nodes[*node_pos & 0xFFFF];

    return 1;
}

#ifdef NON_MATCHING
inline s32 search_both_roots(recomp_tree_t *tree, s32 addr, recomp_node_t **out_node) {
    s32 ret;
    if(addr < tree->code_boundary) {
        ret = treeSearchNode(tree->code_root, addr, out_node);
    } else {
        ret = treeSearchNode(tree->ovl_root, addr, out_node);
    }

    return ret;
}

s32 cpuFindFunction(cpu_class_t *cpu, u32 addr, recomp_node_t **out_node) {
    u32 tree_was_init;
    s32 node_found;
    cpu_dev_t **devs;
    cpu_dev_t *dev;
    u8 *dev_idxs;
    u32 inst;
    s32 fetch_next;
    s32 cur_addr;
    s32 start_addr;
    s32 possible_j_target;
    s32 branch_target;
    s32 delay_flags;
    s32 fwd_branch;
    s32 branch = 0;
    s32 ra_saved = 0;
    s32 branch_dist;
    s32 next_addr;
    s32 jmp_addr;
    u32  no_branch_addr;
    s32 bVar2;

    if(cpu->recomp_tree == NULL) {
        tree_was_init = 0;
        cpu->unk_0x34 = 1;
        if(!xlHeapTake((void**)&cpu->recomp_tree, 0x84)) {
            return 0;
        }
        
        treeInit(cpu, 0x80150002);
    } else {
        tree_was_init = 1;
        
        if(search_both_roots(cpu->recomp_tree, addr, out_node)) {
            cpu->running_node = *out_node;
            return 1;
        }
    }

    dev_idxs = cpu->mem_hi_map;
    devs = cpu->devices;
    start_addr = 0;
    cur_addr = addr;
    branch_target= addr;
    possible_j_target = 0;
    fwd_branch = 0;
    do {
        dev = devs[dev_idxs[addr >> 0x10]];
        dev->lw(dev->dev_obj, cur_addr + dev->addr_offset, (s32*)&inst);
        bVar2 = 1;
        if(!tree_was_init) {
            if(inst != 0 && start_addr == 0) {
                start_addr = cur_addr;
            }
        } else {
            if(start_addr == 0) {
                start_addr = cur_addr;
            }
        }

        fetch_next = 1;
        delay_flags = 0;
        switch(MIPS_OP(inst)) {
            case OPC_SPECIAL:
                switch(SPEC_FUNCT(inst)) {
                        case SPEC_JR:
                            if(!ra_saved && (fwd_branch == 0 || cur_addr > fwd_branch) && (possible_j_target == 0 || cur_addr >= possible_j_target)) {
                                delay_flags = 0x6F;
                            }
                            break;
                        case SPEC_BREAK:
                            if((fwd_branch == 0 || cur_addr > fwd_branch) && (possible_j_target == 0 || cur_addr >= possible_j_target)) {
                                delay_flags = 0x6F;
                                ra_saved = 0;
                            }
                            break;
                        default:
                            fetch_next = lbl_801709C0[SPEC_FUNCT(inst)];
                            break;
                }
                break;
            case OPC_J:
                jmp_addr = (inst & 0x3FFFFFF) << 2 | (cur_addr & 0xF0000000);
                if(jmp_addr >= cur_addr && (jmp_addr - cur_addr) <= 0x1000) {
                    if(possible_j_target == 0) {
                        possible_j_target = jmp_addr;
                    } else if(jmp_addr > possible_j_target) {
                        possible_j_target = jmp_addr;
                    }
                }
                break;
            case OPC_REGIMM:
                switch(REGIMM_SUB(inst)) {
                    case REGIMM_BLTZ:
                    case REGIMM_BGEZ:
                    case REGIMM_BLTZL:
                    case REGIMM_BGEZL:
                    case REGIMM_BLTZAL:
                    case REGIMM_BGEZAL:
                    case REGIMM_BLTZALL:
                    case REGIMM_BGEZALL:
                        branch_dist = MIPS_IMM(inst) * 4;
                        if(branch_dist < 0) {
                            if(tree_was_init == 1 && (cur_addr + (branch_dist + 4)) < branch_target) {
                                start_addr = 0;
                                fwd_branch = 0;
                                possible_j_target = 0;
                                branch = 1;
                                next_addr = branch_target = (cur_addr + (branch_dist + 4));
                                continue;
                            }
                        } else {
                            if(fwd_branch == 0) {
                                fwd_branch = cur_addr + branch_dist;
                            } else {
                                if(cur_addr + branch_dist > fwd_branch) {
                                    fwd_branch = cur_addr + branch_dist;
                                }
                            }
                        }
                        break;
                    default:
                        fetch_next = lbl_80170A00[REGIMM_SUB(inst)];
                        break;
                }
                break;
            case OPC_BEQ:
            case OPC_BEQL:
                branch_dist = MIPS_IMM(inst) * 4;
                if(branch_dist < 0) {
                    if(tree_was_init == 1 && ((cur_addr + branch_dist + 4) < branch_target)) {
                        start_addr = 0;
                        fwd_branch = 0;
                        possible_j_target = 0;
                        branch = 1;
                        branch_target = (cur_addr + branch_dist + 4);
                        continue;
                    }

                    no_branch_addr = cur_addr + 8;
                    dev = devs[dev_idxs[no_branch_addr >> 0x10]];
                    dev->lw(dev->dev_obj, no_branch_addr + dev->addr_offset, (s32*)&inst);
                    if(inst == 0) {
                        do {
                            cur_addr = no_branch_addr;
                            no_branch_addr += 4;
                            dev = devs[dev_idxs[no_branch_addr >> 0x10]];
                            dev->lw(dev->dev_obj, no_branch_addr + dev->addr_offset, (s32*)&inst);
                         } while(inst == 0);

                        if(MIPS_OP(inst) != OPC_LW) {
                            cur_addr -= 4;
                            if((fwd_branch == 0 || cur_addr > fwd_branch) && (possible_j_target == 0 || cur_addr >= possible_j_target)) {
                                delay_flags = 0x6F;
                                ra_saved = 0;
                            }
                        }
                    }
                } else {
                    if(fwd_branch == 0) {
                        fwd_branch = cur_addr + branch_dist;
                    }else if(fwd_branch < cur_addr + branch_dist) {
                        fwd_branch = cur_addr + branch_dist;
                    }
                }
                break;
            case OPC_BNE:
            case OPC_BLEZ:
            case OPC_BGTZ:
            case OPC_BNEL:
            case OPC_BLEZL:
            case OPC_BGTZL:
                branch_dist = MIPS_IMM(inst) * 4;
                if(branch_dist < 0) {
                    if(tree_was_init == 1) {
                        next_addr = cur_addr + branch_dist + 4;
                        if(next_addr < branch_target) {
                            start_addr = 0;
                            fwd_branch = 0;
                            possible_j_target = 0;
                            branch = 1;
                            branch_target = next_addr;
                            continue;
                        }
                    }
                } else {
                    if(fwd_branch == 0) {
                        fwd_branch = cur_addr + branch_dist;
                    } else {
                        if(fwd_branch < cur_addr + branch_dist) {
                            fwd_branch = cur_addr + branch_dist;
                        }
                    }
                }
                break;
            case OPC_CP0:
                switch(inst & 0x3F) {
                    case 0x18:
                        if((fwd_branch == 0 || cur_addr > fwd_branch) && (possible_j_target == 0 || cur_addr >= possible_j_target)) {
                            delay_flags = 0xDE;
                            ra_saved = 0;
                        }
                        break;
                    default:
                        if(((inst >> 21) & 0x1F) == 8 && ((inst >> 16) & 0x1F) < 4) {
                            branch_dist = MIPS_IMM(inst) * 4;
                            if(branch < 0) {
                                if(tree_was_init == 1) {
                                    next_addr = cur_addr + branch_dist + 4;
                                    if(next_addr < branch_target) {
                                        start_addr = 0;
                                        fwd_branch = 0;
                                        possible_j_target = 0;
                                        branch = 1;
                                        branch_target = next_addr;
                                        continue;
                                    }
                                }
                            } else {
                                if(fwd_branch == 0) {
                                    fwd_branch = cur_addr + branch_dist;
                                } else if(fwd_branch < cur_addr + branch_dist) {
                                    fwd_branch = cur_addr + branch_dist;
                                }
                            }
                        }
                        break;
                    case 1:
                    case 2:
                    case 5:
                    case 8:
                        break;
                }
                break;
            case OPC_CP1:
                if(((inst >> 21) & 0x1F) == 8 && ((inst >> 16) & 0x1F) < 4) {
                    branch_dist = MIPS_IMM(inst) * 4;
                    if(branch_dist < 0) {
                        if(tree_was_init == 1) {
                            next_addr = cur_addr + branch_dist + 4;
                            if(next_addr < branch_target) {
                                start_addr = 0;
                                fwd_branch = 0;
                                possible_j_target = 0;
                                branch = 1;
                                branch_target = next_addr;
                                continue;
                            }
                        }
                    } else {
                        if(fwd_branch == 0) {
                            fwd_branch = cur_addr + branch_dist;
                        } else if(fwd_branch < cur_addr + branch_dist) {
                            fwd_branch = cur_addr + branch_dist;
                        }
                    }
                }
                break;
            case OPC_LW:
                if(MIPS_RT(inst) != MREG_RA) {
                    break;
                }

                ra_saved = 0;
                if(!tree_was_init || !branch) {
                    break;
                }

                while(1) {
                    dev = devs[dev_idxs[branch_target >> 0x10]];
                    dev->lw(dev->dev_obj, branch_target + dev->addr_offset, (s32*)&inst);
                    if(MIPS_OP(inst) == OPC_SW && MIPS_RT(inst) == MREG_RA) {
                        break;
                    }
                    branch_target -= 4;
                }

                do {
                    next_addr = branch_target;
                    branch_target -= 4;
                    dev = devs[dev_idxs[branch_target >> 16]];
                    dev->lw(dev->dev_obj, branch_target + dev->addr_offset, (s32*)&inst);
                    if(inst != 0) {
                        fwd_branch = next_addr - 8;

                        if(search_both_roots(cpu->recomp_tree, fwd_branch, out_node)) {
                            break;
                        }

                        if(node_found) {
                            break;
                        }
                    }
                } while(inst != 0);

                start_addr = 0;
                fwd_branch = 0;
                possible_j_target = 0;
                branch = 0;
                branch_target = next_addr;
                continue;
            case OPC_SW:
                if(MIPS_RT(inst) == MREG_RA) {
                    ra_saved = 1;
                }
                break;
            default:
                fetch_next = lbl_80170980[MIPS_OP(inst)];
                break;
        }

        next_addr = cur_addr;
        if(delay_flags != 0) {
            if(delay_flags == 0x6F) {
                next_addr += 8;
                cur_addr += 4;
            } else {
                next_addr += 4;
            }

            if(tree_was_init) {
                if(gSystem->rom_id == 'NM8E') {
                    if(cur_addr == 0x802F1FF0) {
                        start_addr = 0x802F1F50;
                    } else if(cur_addr == 0x80038308) {
                        start_addr = 0x800382F0;
                    }
                } else if(gSystem->rom_id == 'NMFE') {
                    if(cur_addr == 0x8009E420) {
                        start_addr = 0x8009E380;
                    }
                } else if(gSystem->rom_id == 'NMQE' || gSystem->rom_id == 'NMQJ' || gSystem->rom_id == 'NMQP') {
                    if(start_addr == 0x802C88FC) {
                        cur_addr = 0x802C8974;
                    } else if(start_addr = 0x802C8978) {
                        cur_addr = 0x802C8A5C;
                    } else if(start_addr == 0x802C8A60) {
                        cur_addr = 0x802C8C60;
                    }
                }
            }

            if(!treeInsert(cpu, start_addr, cur_addr)) {
                return 0;
            }

            if(tree_was_init) {
                if(!search_both_roots(cpu->recomp_tree, addr, out_node)) {
                    return 0;

                }

                cpu->running_node = *out_node;
                return 1;
            }

            start_addr = 0;
            fwd_branch = 0;
            possible_j_target = 0;
            bVar2 = 0;

            if(!tree_was_init && cpu->recomp_tree->node_cnt > 0xF82) {
                fetch_next = 0;
            }
        }

        if(bVar2) {
            next_addr += 4;
        }

        cur_addr = next_addr;
    } while(fetch_next);

    if(tree_was_init) {
        return 0;
    }

    treeInsert(cpu, 0x80000180, 0x8000018C);
    if(cpu->recomp_tree->n64_end <= 0x80000180) {
        treeSearchNode(cpu->recomp_tree->ovl_root, 0x80000180, out_node);
    } else {
        treeSearchNode(cpu->recomp_tree->code_root, 0x80000180, out_node);
    }

    (*out_node)->unk_0x28 = 0;

    if(!search_both_roots(cpu->recomp_tree, addr, out_node)) {
        return 0;
    }

    cpu->running_node = *out_node;
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuFindFunction.s")
#endif

s32 func_8003E604(cpu_class_t *cpu, s32 arg1, s32 arg2) {
    recomp_tree_t *tree = cpu->recomp_tree;
    s32 amt;

    if(tree == NULL) {
        return 1;
    }

    if(arg1 < tree->code_boundary && arg2 > tree->code_boundary) {
        treeAdjustRoot(cpu, arg1, arg2);
    }

    if(tree->unk_0x70 != 0 && tree->unk_0x7C != NULL) {
        s32 ret = 0;
        if(arg1 <= tree->unk_0x7C->n64_start_addr) {
            if (arg2 >= tree->unk_0x7C->n64_end_addr || arg2 >= tree->unk_0x7C->n64_start_addr) {
                ret = 1;
            }
        } else if(arg2 >= tree->unk_0x7C->n64_end_addr) {
            if(arg1 <= tree->unk_0x7C->n64_start_addr || arg1 <= tree->unk_0x7C->n64_end_addr) {
                ret = 1;
            }
        }
        
        if(ret) { 
            tree->unk_0x7C = NULL;
            tree->unk_0x80 = 0;
        }
    }

    if(arg1 < tree->code_boundary) {
        do {
            amt = treeKillRange(cpu, tree->code_root, arg1, arg2);
            tree->node_cnt -= amt;
        } while(amt != 0);
    } else {
        do {
            amt = treeKillRange(cpu, tree->ovl_root, arg1, arg2);
            tree->node_cnt -= amt;
        } while(amt != 0);
    }
    
    return 1;
}
