#include "cpu.h"
#include "mips.h"
#include "rom.h"
#include "xlHeap.h"
#include "system.h"

int cpuGetAddressBuffer(cpu_class_t *cpu, void **buffer, u32 addr);
int cpuFreeCachedAddress(cpu_class_t *cpu, s32, s32);
int func_8003E604(cpu_class_t *cpu, s32, s32);
int rspGetBuffer(void *, void **, u32, void *);
int func_8000E054(cpu_class_t *cpu, u32, u32, u32, u32);
int func_8000CB1C(cpu_class_t *cpu);
int cpuMakeDevice(cpu_class_t *cpu, u32*, void *, s32, s32, u32, u32);
void DCStoreRange(void *, size_t);
void ICInvalidateRange(void *, size_t);
double sqrt(double in);
double ceil(double in);
double floor(double in);

extern class_t lbl_80171F38;
extern u32 reg_map[32];
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

u64 lbl_80170780[] = { 
    0x000000008000003F, 0x000000000000003F, 0x000000003FFFFFFF, 0x000000003FFFFFFF,
    0xFFFFFFFFFFFFFFF0, 0x0000000001FFE000, 0x000000000000001F, 0x0000000000000000,
    0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFE0FF, 0x00000000FFFFFFFF,
    0x00000000FFFFFFFF, 0x00000000F000FF7C, 0xFFFFFFFFFFFFFFFF, 0x000000000000FFFF,
    0x00000000FFFFEFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFB, 0x000000000000000F,
    0x00000000FFFFFFF0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000, 0x00000000000000FF, 0x00000000FFBFFFFF,
    0x00000000FFFFFFFF, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000
};

u64 lbl_80170880[] = {
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
    0x8F480018, 0x11000014, 0x00000000, 0x4448F800,
    0x00000000, 0x8CBB0018, 0x1360000A, 0x00000000,
    0x445BF800, 0x00000000, 0x13600009, 0xACBF011C,
    0x445BF800, 0x8F5B0018, 0x13600013, 0x00000000,
    0x8F5B012C, 0x44DBF800
};

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000CB1C.s")

int cpuFreeCachedAddress(cpu_class_t *cpu, s32 addr_start, s32 addr_end) {
    s32 i;
    s32 j;
    recomp_cache_t *recomp_cache = cpu->recomp_cache;

    i = 0;
    while(i < cpu->cache_cnt) {
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

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuFindCachedAddress.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuTestInterrupt.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuException.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuMakeDevice.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000D580.s")

s32 cpuSetTLB(cpu_class_t *cpu, s32 arg1);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuSetTLB.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000DEBC.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000DF58.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E054.s")
extern u64 lbl_80170880[];
s32 cpuSetCP0Status(cpu_class_t *, u32, u64, u32);
#ifdef NON_MATCHING
s32 func_8000E0E8(cpu_class_t *cpu, s32 reg, s64 val) {
    s32 set_reg = 0;

    switch(reg) {
        case 0:
            cpu->cp0[0].d = (val & lbl_80170880[0]) | (cpu->cp0[0].d & 0x80000000);
            break;
        case 1:
        case 7:
        case 8:
        case 15:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 27:
        case 31:
            break;
        case 9:
            set_reg = 1;
            break;
        case 11:
            set_reg = 1;
            xlObjectEvent(gSystem, 0x1001, (void*)3);
            if(cpu->status & 1 || !(val & lbl_80170880[11])) {
                cpu->status &= ~1;
            } else {
                cpu->status |= 1;
            }

            break;
        case 12:
            cpuSetCP0Status(cpu, lbl_80170880[12], val & lbl_80170880[12], 0);
            break;
        case 13:
            xlObjectEvent(gSystem, val & 0x100 ? 0x1000 : 0x1001, (void*) 0);
            xlObjectEvent(gSystem, val & 0x200 ? 0x1000 : 0x1001, (void*) 0);
            set_reg = 1;
            break;
        case 14:
            set_reg = 1;
            break;
        case 16:
            cpu->cp0[0x10].d = (u32)((u32)val & (u32)lbl_80170880[0x10]);
            break;
        default:
            set_reg = 1;
            break;
    }

    if(set_reg) {
        cpu->cp0[reg].d = val & lbl_80170880[reg];
    }

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E0E8.s")
#endif

extern u64 lbl_80170780[];
#ifdef NON_MATCHING
s32 cpuGetRegisterCP0(cpu_class_t *cpu, s32 reg, s64 *dest) {
    s32 i;
    s32 res;
    s32 set_reg = 0;

    switch(reg) {
        case 1: // RANDOM
            for(i = 0, res = 0; i < 0x30; i++) {
                if(!(cpu->unk_0x248[i].unk_0x10.d & 2)) {
                    res++;
                }
            }
            *dest = res;
            break;
        case 7: // rsvd?
            *dest = 0;
            break;
        case 8:
            set_reg = 1;
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
        case 31:
            *dest = 0;
            break;
        default:
            set_reg = 1;
    }

    if(set_reg) {
        *dest = cpu->cp0[reg].d & lbl_80170780[reg];
    }

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuGetRegisterCP0.s")
#endif

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

int __cpuERET(cpu_class_t *cpu) {
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

int __cpuBreak(cpu_class_t *cpu) {
    cpu->status |= 2;
    return 1;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E4B8.s")

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
                code[(*pos)++] = 0x80A30000 + ((u32)&cpu->unk_0x30 - (u32)cpu); // addic r0, r0, 0xA380
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
                // addic r0, r0, offset(cpu_class_t, unk_0x30)
                code[(*pos)++] = 0x80A30000 + ((u32)&cpu->unk_0x30 - (u32)cpu);
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                            cpu->unk_0x12278 = MIPS_RD(cur_inst);
                            cpu->unk_0x1227C = rA;
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                            cpu->unk_0x12278 = MIPS_RD(cur_inst);
                            cpu->unk_0x1227C = rA;
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                            cpu->unk_0x12278 = MIPS_RD(cur_inst);
                            cpu->unk_0x1227C = rA;
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                            cpu->unk_0x12278 = MIPS_RD(cur_inst);
                            cpu->unk_0x1227C = rA;
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                            cpu->unk_0x12278 = MIPS_RD(cur_inst);
                            cpu->unk_0x1227C = rA;
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                                    // mr r6, cpu->unk_0x1227C
                                    code[(*pos)++] = 0x7C060378 | (cpu->unk_0x1227C << 0xB) | (cpu->unk_0x1227C << 0x15);
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
                            cpu->unk_0x12278 = MIPS_RD(cur_inst);
                            cpu->unk_0x1227C = rA;
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
                            // stw r7, cpu->unk_0x30
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
                }
                break;
        }
    }
}

//#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuGetPPC.s")

int func_80031D4C(cpu_class_t *cpu, recomp_node_t *node, s32 arg2) {
    return 0;
}

s32 cpuFindFunction(cpu_class_t *cpu, s32 addr, recomp_node_t **out_node);
s32 libraryTestFunction(void *library, recomp_node_t *node);

#ifdef NON_MATCHING
s32 cpuMakeFunction(cpu_class_t *cpu, recomp_node_t **out_node, s32 addr) {
    s32 success = 1;
    unk_node_0x0C unk0xC[0x400];
    s32 pos;
    s32 total_inst;
    s32 i;
    size_t code_size;
    size_t ref_size;
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
        node->unk_0x08 = 0;
        node->unk_0x0C = unk0xC;
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
        if(node->unk_0x1C != 0) {
            ref_size = node->unk_0x1C * 8;
            alloc_size += ref_size;
        } else {
            ref_size = 0;
        }

        if(node->unk_0x08 > 0) {
            alloc_size += node->unk_0x08 * sizeof(*node->unk_0x0C);
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
                clean_cnt = cpu->unk_0x34 - 300;
                continue;
            }

            clean_cnt += 95;
            if(clean_cnt > (cpu->unk_0x34 - 10)) {
                clean_cnt = cpu->unk_0x34 - 10;
            }

            recomp_tree->unk_0x70 = 0;
            recomp_tree->unk_0x7C = NULL;
            recomp_tree->unk_0x80 = 0;
            if(node != NULL && node->unk_0x28 > 0) {
                node->unk_0x28 = cpu->unk_0x34;
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
        if(ref_size != 0) {
            unk_node_0x18 *unk18;
            node->unk_0x18 = (unk_node_0x18*)((char*)code_p + code_size);
            unk18 = node->unk_0x18;
            for(i = 0; i < ref_cnt; i++) {
                unk18[i].unk_0x00 = 0;
                unk18[i].unk_0x04 = 0;
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
        if(node->unk_0x08 > 0) {
            if(node->unk_0x08 >= 0x400) {
                return 0;
            }

            node->unk_0x0C = (unk_node_0x0C*)((char*)code + code_size + ref_size);
            for(i = 0; i < node->unk_0x08; i++) {
                node->unk_0x0C[i] = unk0xC[i];
            }
        } else {
            node->unk_0x0C = NULL;
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

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_80032248.s")

inline s32 find_in_node_unk_0xC(recomp_node_t *node, s32 pc) {
    int i;

    for(i = 0; i < node->unk_0x08; i++) {
        if(pc == node->unk_0x0C[i].unk_0x04) {
            return 0;
        }
    }

    return 1;
}

int func_800326D0(cpu_class_t *cpu, s32 pc, s32 r5) {
    if(!cpu->unk_0x12270) {
        return 0;
    }

    if(cpu->unk_0x12274 == 0) {
        return 0;
    }

    if(r5 != cpu->unk_0x12278) {
        return 0;
    }

    if(!find_in_node_unk_0xC(cpu->running_node, pc)) {
        cpu->unk_0x12274 = 0;
        return 0;
    }

    cpu->unk_0x12274 = 0;
    return 1;
}

int func_80032780(cpu_class_t *cpu, s32 pc, s32 r5) {
    if(!cpu->unk_0x12270) {
        return 0;
    }

    if(cpu->unk_0x12280 == 0) {
        return 0;
    }

    if(r5 != cpu->unk_0x12284) {
        return 0;
    }

    if(!find_in_node_unk_0xC(cpu->running_node, pc)) {
        cpu->unk_0x12280 = 0;
        return 0;
    }

    cpu->unk_0x12280 =0;
    return 1;
}

int func_80032830(cpu_class_t *cpu, s32 pc, u32 *code, s32 base, s32 rt) {
    s32 i;

    if(code == NULL) {
        return 0;
    }

    if(!cpu->unk_0x12270) {
        return 0;
    }

    if(!find_in_node_unk_0xC(cpu->running_node, pc)) {
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

void func_8007FCD0();

#ifdef NON_MATCHING
int func_800328EC(cpu_class_t *cpu, u32 addr, s32 inst, u32 *code, u32 *pos) {
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

    if(!find_in_node_unk_0xC(cpu->running_node, addr)) {
        return 0;
    }

    switch(inst >> 0x1A) {
        case 0xD:
            if(cpu->unk_0x12278 == ((inst >> 0x15) & 0x1F) && ((inst >> 0x15) & 0x1F) == ((inst >> 0x10) & 0x1F)) {
                if(cpu->unk_0x12290 != 1000) {
                    return 0;
                }
                code[*pos - 1] = 0x60000000;
                code[(*pos)++] = (inst & 0xFFFF) | (cpu->unk_0x1227C << 0x10) | (cpu->unk_0x1227C << 0x15) | 0x60000000;
                code[(*pos)++] = 0x60000000;
                code[(*pos)++] = (((inst >> 0x10) & 0x1F) * 8) + ((cpu->unk_0x1227C << 0x15) | 0x90030000) + ((u32)&cpu->gpr[0] - (u32)cpu);
                cpu->unk_0x12274 = 2;
                return 1;
            }
            return 0;
        case 9:
            if(cpu->unk_0x12278 == ((inst >> 0x15) & 0x1F) && ((inst >> 0x15) & 0x1F) == ((inst >> 0x10) & 0x1F)) {
                if(cpu->unk_0x12290 != 1000) {
                    return 0;
                }

                code[*pos - 1] = 0x60000000;
                code[(*pos)++] = (inst & 0xFFFF) | (cpu->unk_0x1227C << 0x10) | (cpu->unk_0x1227C << 0x15) | 0x38000000;
                code[(*pos)++] = 0x60000000;
                code[(*pos)++] = (((inst >> 0x10) & 0x1F) * 8) + ((cpu->unk_0x1227C << 0x15) | 0x90030000) + ((u32)&cpu->gpr[0] - (u32)cpu);
                cpu->unk_0x12274 = 2;
                return 1;
            }
            return 0;
        default:
            func_8007FCD0();
            return 0;
    }

    return 0;

}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_800328EC.s")
#endif

void func_80032B74(u32 arg0) {
    gSystem->cpu->unk_0x3C = arg0;
    if(lbl_8025CFE8 != NULL) {
        lbl_8025CFE8();
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteUpdate.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DSLLV.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DSRLV.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DSRAV.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DMULT.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DMULTU.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DDIV.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_DDIVU.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_S_SQRT.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_D_SQRT.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_W_CVT_SD.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_L_CVT_SD.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_CEIL_W.s")

#ifdef NON_MATCHING

inline u32 PPC_STWU(u32 rS, u32 rA, s16 offset) {
    return (0x25 << 0x1A) | ((rS & 0x1F) << 0x15) | ((rA & 0x1F) << 0x10) | (offset & 0xFFFF);
}

inline u32 PPC_LFD(u32 rD, u32 rA, s16 offset) {
    return (0x32 << 0x1A) | ((rD & 0x1F) << 0x15) | ((rA & 0x1F) << 0x10) | (offset & 0xFFFF);
}

inline u32 PPC_FCMPO(u32 crfD, u32 frA, u32 frB) {
    return (0x3F << 0x1A) | ((crfD & 7) << 0x15) | ((frA & 0x1F) << 0x10) | ((frB & 0x1F) << 0x10) | (0x20 << 1);
}

inline u32 PPC_BC(u32 BO, u32 BI, s32 target) {
    return (0x10 << 0x1A) | ((BO & 0x1F) << 0x15) | ((BI & 0x1F) << 0x10) | (target & 0x3FFC);
}

inline u32 PPC_BLT(s32 target) {
    return PPC_BC(12, 0, target);
}

inline u32 PPC_ADDI(u32 rD, u32 rA, s16 imm) {
    return (0xE << 0x1A) | ((rD & 0x1F) << 0x15) | ((rA & 0x1F) << 0x10) | (imm & 0xFFFF);
}

inline u32 PPC_FCTIWZ(u32 frD, u32 frB, u32 rC) {
    return (0x3F << 0x1A) | ((frD & 0x1F) << 0x15) | ((frB & 0x1F) << 0xB) | (0xF << 1) | ((rC & 1) << 0);
}

inline u32 PPC_STFD(u32 frS, u32 rA, s16 offset) {
    return (0x36 << 0x1A) | ((frS & 0x1F) << 0x15) | ((rA & 0x1F) << 0x10) | (offset & 0xFFFF);
}

inline u32 PPC_LWZ(u32 rD, u32 rA, s16 offset) {
    return (0x20 << 0x1A) | ((rD & 0x1F) << 0x15) | ((rA & 0x1F) << 0x10) | (offset & 0xFFFF);
}

inline u32 PPC_SUBF(u32 rD, u32 rA, u32 rB) {
    return (0x1F << 0x1A) | ((rD & 0x1F) << 0x15) | ((rA & 0x1F) << 0x10) | ((rB & 0x1F) << 0xB) | (0x28 << 1);
}

inline u32 PPC_BLR() {
    return (0x13 << 0x1A) | (0x14 << 0x15) | (0x10 << 1);
}


int cpuCompile_FLOOR_W(cpu_class_t *cpu, u32 **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x30000034)) {
        return 0;
    }

    *code = buf;
    buf[0] = PPC_STWU(1, 1, -32);
    buf[1] = PPC_LFD(0, 3, ((s32)&cpu->fpr[0] - (s32)cpu));
    //buf[1] = 0xc8030000 | ((u32)&cpu->fpr[0] - (u32)cpu);
    //buf[2] = 0xfc010040;
    buf[2] = PPC_FCMPO(0, 1, 0);
    //buf[3] = 0x4180000c;
    buf[3] = PPC_BLT(12); 
    //buf[4] = 0x38c00000;
    buf[4] = PPC_ADDI(6, 0, 0);
    //buf[5] = 0x42800008;
    buf[5] = PPC_BC(20, 0, 8);
    //buf[6] = 0x38c00001;
    buf[6] = PPC_ADDI(6, 0, 1);
    buf[7] = PPC_FCTIWZ(1, 1, 0);
    //buf[7] = 0xfc20081e;
    //buf[8] = 0xd8210010;
    buf[8] = PPC_STFD(1, 1, 16);
    //buf[9] = 0x80a10014;
    buf[9] = PPC_LWZ(5, 1, 20);
    //buf[10] = 0x7ca62850;
    buf[10] = PPC_SUBF(5, 6, 5);
    buf[11] = PPC_ADDI(1, 1, 32);
    //buf[11] = 0x38210020;
    //buf[12] = 0x4E800020;
    buf[12] = PPC_BLR();
    DCStoreRange(buf, 0x34);
    ICInvalidateRange(buf, 0x34);
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_FLOOR_W.s")
#endif  

#ifdef NON_MATCHING
int cpuCompile_LB(cpu_class_t *cpu, void **code) {
    u32 *buf;

    if(!xlHeapTake((void**)&buf, 0x3000002C)) {
        return 0;
    }

    *code = buf;
    buf[0] = 0x54c6103a;
    buf[1] = 0x38e30000 | ((u32)&cpu->devices - (u32)cpu);
    buf[2] = 0x7cc6382e;
    buf[3] = 0x80e60008;
    buf[4] = 0x7ca53a14;
    buf[5] = 0x80e60004;
    buf[6] = 0x80e70004;
    buf[7] = 0x7ca538ae;
    buf[8] = 0x7ca50774;
    buf[9] = 0x4e800020;

    DCStoreRange(buf, 0x28);
    ICInvalidateRange(buf, 0x28);

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LB.s")
#endif

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LH.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LW.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LBU.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LHU.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_SB.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_SH.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_SW.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LDC.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_SDC.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LWL.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCompile_LWR.s")

u32 OSGetTick(void);
s32 systemCheckInterrupts(system_class_t *sys);
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


#ifdef NON_MATCHING
inline unkFrameSet(u32 rom_id, u32 *frame, u32 addr) {
    if(rom_id == 'NKTJ') {
        if(addr == 0x802A4118) {
            frame[0x11] = 0;
        }
        if(addr == 0x80072D94) {
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

inline s32 calc_ea(cpu_class_t *cpu, s32 reg, s32 imm) {
    return cpu->gpr[reg].w[1] + (u32)imm;
}

void *cpuExecuteOpcode(cpu_class_t *cpu, s32 arg1, u32 addr, u32 *ret) {
    s32 save_ra = 0;
    u32 tick = OSGetTick();
    u32 *code_buf;
    u32 inst;
    u64 prev_ra;
    u8 *dev_idx;
    cpu_dev_t *dev;
    cpu_dev_t **devs;
    reg64_t tmp;
    s32 i;
    s32 val;
    u32 ea;

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
        prev_ra = cpu->gpr[0x1F].d;
        cpu->gpr[0x1F].w[1] = cpu->unk_0x30;
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
                    cpu->lo.d = cpu->gpr[MIPS_RD(inst)].d;
                    break;
                case SPEC_DSLLV: // DSLLV
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RT(inst)].d << (cpu->gpr[MIPS_RS(inst)].d & 0x1F);
                    break;
                case SPEC_DSRLV: // DSRLV
                    cpu->gpr[MIPS_RD(inst)].d = cpu->gpr[MIPS_RT(inst)].d >> (cpu->gpr[MIPS_RS(inst)].d & 0x1F);
                    break;
                case SPEC_DSRAV: // DRAV
                    cpu->gpr[MIPS_RD(inst)].sd = cpu->gpr[MIPS_RT(inst)].sd >> (cpu->gpr[MIPS_RS(inst)].sd & 0x1F);
                    break;
                case SPEC_MULT: // MULT
                    {
                        s64 rs = cpu->gpr[MIPS_RS(inst)].sw[1];
                        s64 rt = cpu->gpr[MIPS_RT(inst)].sw[1];
                        tmp.sd = (s64)(rs * rt);
                        cpu->lo.sd = (s32)(tmp.sd & 0xFFFFFFFF);
                        cpu->hi.sd = (s32)(tmp.sd >> 32);
                    }
                    break;
                case SPEC_MULTU: // MULTU
                    {
                        u64 rs = cpu->gpr[MIPS_RS(inst)].w[1];
                        u64 rt = cpu->gpr[MIPS_RT(inst)].w[1];
                        tmp.d = (s64)(rs * rt);
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

                        reg64_t *rs = &cpu->gpr[MIPS_RS(inst)];
                        reg64_t *rt = &cpu->gpr[MIPS_RT(inst)];

                        cpu->lo.w[1] = rs->w[1] * rt->w[1];
                        cpu->lo.w[0] = (u32)(((u64)rs->w[1] * (u64)rt->w[1]) >> 32) + (rs->w[0] * rt->w[1]);
                        cpu->hi.sd = -((cpu->lo.w[0] ^ 0x80000000) < 0x80000000);
                    }
                    break;
                case SPEC_DMULTU: // DMULTU
                    {
                        tmp.d = cpu->gpr[MIPS_RS(inst)].d * cpu->gpr[MIPS_RT(inst)].d;
                        cpu->lo.d = tmp.d;
                        cpu->hi.d = tmp.d >> 32;
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
    
                cpu->gpr[0x1F].w[1] = cpu->pc + 4;
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
                    cpu->cp0[2].d = cpu->unk_0x248[cpu->cp0[0].w[1] & 0x3F].unk_0x00.d;
                    cpu->cp0[3].d = cpu->unk_0x248[cpu->cp0[0].w[1] & 0x3F].unk_0x08.d;
                    cpu->cp0[0xA].d = cpu->unk_0x248[cpu->cp0[0].w[1] & 0x3F].unk_0x10.d;
                    cpu->cp0[5].d = cpu->unk_0x248[cpu->cp0[0].w[1] & 0x3F].unk_0x18.d;
                    break;
                case 2: // TLBWI
                    cpuSetTLB(cpu, cpu->cp0[0].w[1] & 0x3F);
                    break;
                case 5: // TLBWR
                    {
                        for(i = 0; i < 0x30; i++) {
                            if(!(cpu->unk_0x248[i].unk_0x10.w[1] & 2)) {
                                val++;
                            }
                        }

                        cpu->cp0[1].sd = val;
                        cpuSetTLB(cpu, val);
                    }
                    break;
                case 8: // TLBP
                    {
                        cpu->cp0[0].d |= 0x80000000;

                        for(i = 0; i < 0x30; i++) {
                            if(cpu->unk_0x248[i].unk_0x00.w[1] & 2) {
                                if(!(cpu->cp0[10].d ^ cpu->unk_0x248[i].unk_0x10.d)) {
                                    cpu->cp0[0].d = i;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case 24: // ERET
                    if(cpu->cp0[12].d & 4) {
                        cpu->pc = cpu->cp0[30].w[1];
                        cpu->cp0[12].d &= ~4;
                    } else {
                        cpu->pc = cpu->cp0[14].w[1];
                        cpu->cp0[12].d &= ~2;
                    }
                    cpu->status |= 0x24;
                    break;
                case 0:
                default:
                    switch((inst >> 0x15) & 0x1F) {
                        case 0: // MFC0
                            {
                                if(!cpuGetRegisterCP0(cpu, (inst >> 0xB) & 0x1F, &tmp.w[1])) {
                                    break;
                                }

                                cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.d & 0xFFFFFFFFUL;
                            }
                            break;
                        case 1: // DMFC0
                            {
                                if(!cpuGetRegisterCP0(cpu, (inst >> 0xB) & 0x1F, &tmp.w[0])) {
                                    break;
                                }

                                cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.d;
                            }
                            break;
                        case 2:
                            break;
                        case 4: // MTC0
                            func_8000E0E8(cpu, (inst >> 0xB) & 0x1F, 0, cpu->gpr[(inst >> 0x10) & 0x1F].w[1]);
                            break;
                        case 5: // DMTC0
                            func_8000E0E8(cpu, (inst >> 0xB) & 0x1F, cpu->gpr[(inst >> 0x10) & 0x1F].w[0], cpu->gpr[(inst >> 0x10) & 0x1F].w[1]);
                            break;
                        case 6:
                            break;
                    }
                    break;
            }
            break;
        case OPC_CP1:
            if(MIPS_FDT(inst) == 0) {
                if(MIPS_FSUB(inst) >= 0x10) {
                    break;
                }

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
                        case 49: // C.F.S
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 50: // C.UN.S
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 51: // C.EQ.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] == cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 52: // C.UEQ.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] == cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 53: // C.OLT.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] < cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 54: // C.ULT.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] < cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 55: // C.OLE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 56: // C.ULE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 57: // C.SF.S
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 58: // C.NGLE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 59: // C.SEQ.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] == cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 60: // C.NGL.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] == cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 61: // C.LT.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] < cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 62: // C.NGE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] < cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 63:  // C.LE.S
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].f[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].f[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 64: // C.NGT.S
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
                        case 49: // C.F.D
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 50: // C.UN.D
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 51: // C.EQ.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd == cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 52: // C.UEQ.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd == cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 53: // C.OLT.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd < cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 54: // C.ULT.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd < cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 55: // C.OLE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 56: // C.ULE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 57: // C.SF.D
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 58: // C.NGLE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 59: // C.SEQ.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd == cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 60: // C.NGL.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd == cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 61: // C.LT.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd < cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 62: // C.NGE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd < cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 63:  // C.LE.D
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].fd <= cpu->fpr[(inst >> 0x10) & 0x1F].fd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 64: // C.NGT.D
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
                        case 49: // C.F.W
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 50: // C.UN.W
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 51: // C.EQ.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] == cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 52: // C.UEQ.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] == cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 53: // C.OLT.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] < cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 54: // C.ULT.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] < cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 55: // C.OLE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 56: // C.ULE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 57: // C.SF.W
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 58: // C.NGLE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 59: // C.SEQ.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] == cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 60: // C.NGL.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] == cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 61: // C.LT.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] < cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 62: // C.NGE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] < cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 63:  // C.LE.W
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sw[1] <= cpu->fpr[(inst >> 0x10) & 0x1F].sw[1]) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 64: // C.NGT.W
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
                        case 49: // C.F.L
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 50: // C.UN.L
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 51: // C.EQ.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd == cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 52: // C.UEQ.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd == cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 53: // C.OLT.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd < cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 54: // C.ULT.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd < cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 55: // C.OLE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 56: // C.ULE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 57: // C.SF.L
                            cpu->fscr[0x1F] &= ~0x00800000;
                            break;
                        case 58: // C.NGLE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 59: // C.SEQ.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd == cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 60: // C.NGL.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd == cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 61: // C.LT.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd < cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 62: // C.NGE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd < cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 63:  // C.LE.L
                            if(cpu->fpr[(inst >> 0xB) & 0x1F].sd <= cpu->fpr[(inst >> 0x10) & 0x1F].sd) {
                                cpu->fscr[0x1F] |= 0x00800000;
                            } else {
                                cpu->fscr[0x1F] &= ~0x00800000;
                            }
                            break;
                        case 64: // C.NGT.L
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
            cpu->gpr[(inst >> 0x10) & 0x1F].d = cpu->gpr[(inst >> 0x15) & 0x1F].sd + MIPS_IMM(inst) ;
            break;
        case 31: // precompiled function.
            if(!func_8005D614(gSystem->unk_0x0058, cpu, MIPS_IMM(inst) )) {
                return NULL;
            }
            break;
        case 26: // LDL
            {
                s32 sh = 0x38;
                u8 buf;
                ea = MIPS_IMM(inst) + cpu->gpr[MIPS_RS(inst)].sw[1];

                do {
                    dev = devs[dev_idx[ea >> 0x10]];

                    if(dev->lb(dev->unk_4, ea + dev->unk_8, (s8*)&buf)) {
                        tmp.d = (u64)buf << sh;
                        cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.d | (cpu->gpr[(inst >> 0x10) & 0x1F].d & ~((u64)0xFF << sh));
                    }

                    ea++;
                    sh -= 8;

                } while((ea - 1) & 7);
            }
            break;
        case 27: // LDR
            {
                s32 sh = 0;
                u8 buf;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst) ;

                do {
                    if(devs[dev_idx[ea >> 0x10]]->lb(devs[dev_idx[ea >> 0x10]]->unk_4, ea + devs[dev_idx[ea >> 0x10]]->unk_8, (s8*)&buf)) {
                        tmp.d = (u64)buf << sh;
                        cpu->gpr[(inst >> 0x10) & 0x1F].d = tmp.d | (cpu->gpr[(inst >> 0x10) & 0x1F].d & ~((u64)0xFF << sh));
                    }
                    ea--;
                    sh += 8;
                } while((ea + 1) & 7);
            }
            break;
        case 39: // LWU
            {
                u32 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lw(dev->unk_4, ea + dev->unk_8, (s32*)&dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].d = dst;
                }
            }
            break;
        case 32: // LB
            {
                s8 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lb(dev->unk_4, ea + dev->unk_8, &dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = dst;
                }
            }
            break;
        case 33: // LH
            {
                s16 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lh(dev->unk_4, ea + dev->unk_8, &dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = dst;
                }
            }
            break;
        case 34: // LWL
            {
                u8 buf;
                s32 sh = 0x18;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    dev = devs[dev_idx[ea >> 0x10]];
                    if(dev->lb(dev->unk_4, ea + dev->unk_8, (s8*)&buf)) {
                        tmp.w[1] = (u32)buf << sh;
                        cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = tmp.w[1] | (cpu->gpr[(inst >> 0x10) & 0x1F].w[1] & ~((u32)0xFF << sh));
                    }
                    ea++;
                    sh -= 8;
                } while((ea - 1) & 3);
            }
            break;
        case 35: // LW
            {
                s32 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lw(dev->unk_4, ea + dev->unk_8, &dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = dst;
                }
            }
            break;
        case 36: // LBU
            {
                u8 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lb(dev->unk_4, ea + dev->unk_8, (s8*)&dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = dst;
                }
            }
            break;
        case 37: // LHU
            {
                u16 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);

                if(func_80052D68(gSystem->frame, &dst, ea)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = dst;
                    break;
                }
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lh(dev->unk_4, ea + dev->unk_8, (s16*)&dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = dst;
                }
            }
            break;
        case 38: // LWR
            {
                u8 buf;
                s32 sh = 0;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    dev = devs[dev_idx[ea >> 0x10]];
                    if(dev->lb(dev->unk_4, ea + dev->unk_8, (s8*)&buf)) {
                        tmp.w[1] = (u32)buf << sh;
                        cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = tmp.w[1] | (cpu->gpr[(inst >> 0x10) & 0x1F].w[1] & ~((u32)0xFF << sh));
                    }
                    ea--;
                    sh += 8;
                } while((ea + 1) & 3);
            }
            break;
        case 40: // SB
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                dev->sb(dev->unk_4, ea + dev->unk_8, (s8*)&cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]);
            }
            break;
        case 41: // SH
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                dev->sh(dev->unk_4, ea + dev->unk_8, (s16*)&cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]);
            }
            break;
        case 42: // SWL
            {
                s8 buf;
                s32 sh = 0x18;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    buf = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] >> sh;
                    dev = devs[dev_idx[ea >> 0x10]];
                    dev->sb(dev->unk_4, ea + dev->unk_8, &buf);
                    ea++;
                    sh -= 8;
                } while((ea - 1) & 3);
            }
            break;
        case 43: // SW
            {
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                dev->sw(dev->unk_4, ea + dev->unk_8, &cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]);
            }
            break;
        case 44: // SDL
            {
                s8 buf;
                s32 sh = 0x38;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    buf = cpu->gpr[(inst >> 0x10) & 0x1F].d >> sh;
                    dev = devs[dev_idx[ea >> 0x10]];
                    dev->sb(dev->unk_4, ea + dev->unk_8, &buf);
                    ea++;
                    sh -= 8;
                } while((ea - 1) & 7);
            }
            break;
        case 45: // SDR
            {
                s8 buf;
                s32 sh = 0;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    buf = cpu->gpr[(inst >> 0x10) & 0x1F].d >> sh;
                    dev = devs[dev_idx[ea >> 0x10]];
                    dev->sb(dev->unk_4, ea + dev->unk_8, &buf);
                    ea--;
                    sh += 8;
                } while((ea + 1) & 7);
            }
            break;
            break;
        case 46: // SWR
            {
                s8 buf;
                s32 sh = 0;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + MIPS_IMM(inst);

                do {
                    buf = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] >> sh;
                    dev = devs[dev_idx[ea >> 0x10]];
                    dev->sb(dev->unk_4, ea + dev->unk_8, &buf);
                    ea--;
                    sh += 8;
                } while((ea + 1) & 3);
            }
            break;
        case 48: // LL
            {
                s32 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lw(dev->unk_4, ea + dev->unk_8, &dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = dst;
                }
            }
            break;
        case 49: // LWC1
            {
                s32 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->lw(dev->unk_4, ea + dev->unk_8, &dst)) {
                    if((inst >> 0x10) & 1) {
                        cpu->fpr[MIPS_FS(inst) - 1].d = cpu->fpr[MIPS_FS(inst) - 1].d & 0xFFFFFFFFUL;
                        cpu->fpr[MIPS_FS(inst) - 1].d |= ((u64)cpu->gpr[MIPS_RT(inst)].w[1] << 32);
                    } else {
                        cpu->fpr[(inst >> 0x10) & 0x1F].sw[1] = dst;
                    }
                }
            }
            break;
        case 52: // LLD
            {
                s64 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->ld(dev->unk_4, ea + dev->unk_8, &dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].d = dst;
                }
            }
            break;
        case 53: // LDC1 
            {
                s64 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->ld(dev->unk_4, ea + dev->unk_8, &dst)) {
                    cpu->fpr[(inst >> 0x10) & 0x1F].d = dst;
                }
            }
            break;
        case 55: // LD
            {
                s64 dst;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if(dev->ld(dev->unk_4, ea + dev->unk_8, &dst)) {
                    cpu->gpr[(inst >> 0x10) & 0x1F].d = dst;
                }
            }
            break;
        case 56: // SC
            {
                ea = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];
                
                cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = !!dev->sw(dev->unk_4, ea + dev->unk_8, &cpu->gpr[(inst >> 0x15) & 0x1F].sw[1]);
            }
            break;
        case 57: // SWC1
            {
                u32 buf;
                
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                if((inst >> 0x10) & 1) {
                    buf = cpu->fpr[(inst >> 0x10) & 0x1F].w[0];
                } else {
                    buf = cpu->fpr[(inst >> 0x10) & 0x1F].w[1];
                }

                dev->sw(dev->unk_4, ea + dev->unk_8, (s32*)&buf);
            }
            break;
        case 60: // SCD
            {
                ea = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];
                
                cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = !!dev->sd(dev->unk_4, ea + dev->unk_8, &cpu->gpr[(inst >> 0x15) & 0x1F].sd);
            }
            break;
        case 61: // SDC1
            {
                u64 buf  = cpu->fpr[(inst >> 0x10) & 0x1F].d;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                dev->sd(dev->unk_4, ea + dev->unk_8, (s64*)&buf);
            }
            break;
        case 63:
            {
                u64 buf  = cpu->gpr[(inst >> 0x10) & 0x1F].d;
                ea = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + MIPS_IMM(inst);
                dev = devs[dev_idx[ea >> 0x10]];

                dev->sd(dev->unk_4, ea + dev->unk_8, (s64*)&buf);
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

s32 videoForceRetrace(void *);
s32 cpuExecuteUpdate(cpu_class_t *cpu, u32 **, u32);

#ifdef NON_MATCHING
void *cpuExecuteIdle(cpu_class_t *cpu, u32 r4, u32 addr, u32 *ret) {
    rom_class_t *rom;
    u32 tick;

    rom = gSystem->rom;
    tick = OSGetTick();

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

inline void findUnk0x18(cpu_class_t *cpu, s32 addr, u32 *ret) {
    s32 i;
    recomp_node_t *node;
    unk_node_0x18 *unk_0x18;
    u32 *ret_p = ret - 1;
    
    cpu->unk_0x34++;
    cpuFindFunction(cpu, cpu->unk_0x30 - 8, &node);

    unk_0x18 = node->unk_0x18;
    for(i = 0; i < node->unk_0x1C; unk_0x18++, i++) {
        if(addr == unk_0x18->unk_0x00) {
            if(unk_0x18->unk_0x04 == NULL) {
                node->unk_0x18[i].unk_0x04 = ret_p;
                return;
            }
        }
    }
}

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

    findUnk0x18(cpu, addr, ret);

    flg = (reg_map[MREG_RA] & 0x100) != 0;
    ret_p = ret - (!!flg + 3);
    if(flg) {
        ret_p[0] = ((u32)ret >> 0x10) | (5 << 0x15) | (0 << 0x10) | 0x3C000000;
        ret_p[1] = ((u32)ret & 0xFFFF) |(5 << 0x15) | (5 << 0x10) | 0x60000000;
        DCStoreRange(ret_p, 8);
        ICInvalidateRange(ret_p, 8);
    } else {
        u32 reg = reg_map[MREG_RA];
        ret_p[0] = ((u32)ret >> 0x10) | (reg << 0x15) | (0 << 0x10) | 0x3C000000;
        ret_p[1] = ((u32)ret & 0xFFFF) | (reg << 0x15) | 0x60000000 | (reg << 0x10);  
        DCStoreRange(ret_p, 8);
        ICInvalidateRange(ret_p, 8);
    }

    if(!cpuExecuteUpdate(cpu, &ret, tick)) {
        return NULL;
    }

    offset = (u32)ret - (u32)&ret_p[3];
    if(flg) {
        ret_p[3] = (offset & 0x3FFFFFC) | 0x48000000;
        DCStoreRange(ret_p, 16);
        ICInvalidateRange(ret_p, 16);
    } else {
        ret_p[2] = (offset & 0x3FFFFFC) | 0x48000000;
        DCStoreRange(ret_p, 12);
        ICInvalidateRange(ret_p, 12);
    }

    cpu->tick = OSGetTick();

    return ret;
}

void *cpuExecuteLoadStore(cpu_class_t *cpu, u32 r4, u32 addr, u32 *ret);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteLoadStore.s")

void *cpuExecuteLoadStoreF(cpu_class_t *cpu, u32 r4, u32 addr, u32 *ret);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteLoadStoreF.s")

int cpuMakeLink(cpu_class_t *cpu, u32 **link_code, void *handler) {
    u32 *code;
    s32 i;
    u32 paddr;
    s32 cnt = sizeof(reg_map) / sizeof(*reg_map);
    
    if(!xlHeapTake((void**)&code, 0x30000200)) {
        return 0;
    }

    *link_code = code;
    *code++ = 0x7CC802A6; // mflr r6

    for(i = 1; i < 32; i++) {
        if(!(reg_map[i] & 0x100)) {
            *code++ = (((u32)&cpu->gpr[i] - (u32)cpu) + 4) |
                      (reg_map[i] << 0x15) |
                      0x90030000;                               // stw rx, off(r3)
        }
    }

    *code++ = (((u32)handler - (u32)code) & 0x3FFFFFC) |
              0x48000001;                                       // bl handler
    *code++ = 0x7C6803A6;                                       // mtlr r3
    *code++ = ((u32)cpu >> 0x10) |
              0x3C600000;                                       // lis r3, cpu@h
    *code++ = ((u32)cpu & 0xFFFF) |
              0x60630000;                                       // ori r3, r3, cpu@l
    *code++ = (((s32)&cpu->unk_0x34 - (s32)cpu)) +
              0x80830000;                                       // lwz r4, 0x34(r3)

    paddr = (u32)gSystem->ram->dram - 0x80000000;
    *code++ = (paddr >> 0x10) |
              0x3D000000;                                       // lui r8, dram@h
    if(cpu->unk_0x12224 & 0x100) {
        *code++ = 0x3D20DFFF;                                   // lis r9, 0xDFFF
        *code++ = (paddr & 0xFFFF) |
                  0x61080000;                                   // ori r8, r8, dram@l
        *code++ = 0x6129FFFF;                                   // ori r9, r9, 0xFFFF
    } else if(cpu->unk_0x12224 & 1) {
        *code++ = (((u32)cpu->mem_hi_map - (u32)cpu)) +
                  0x39230000;                                   // addi r9, r3, 0xF60
        *code++ = (paddr & 0xFFFF) |
                  0x61080000;                                   // ori r8, r8, dram@l
    }

    *code++ = (reg_map[0] << 0x15) |
              0x38000000;                                       // li r10, 0

    for(i = 1; i < 32; i++) {
        if(!(reg_map[i] & 0x100)) {
            *code++ = (((u32)&cpu->gpr[i] - (u32)cpu) + 4) |
                      (reg_map[i] << 0x15) |
                      0x80030000;                               // lwz rx, off(r3)
        }
    }

    *code++ = 0x4E800020;                                       // blr

    DCStoreRange(*link_code, 0x200);
    ICInvalidateRange(*link_code, 0x200);

    return 1;
}

#ifdef NON_MATCHING
extern void (*lbl_8025CFE8)(u32);

inline s32 __free(void **ptr) {
    void *tmp = ptr;
    if(!xlHeapFree(&tmp)) {
        return 0;
    }

    *ptr = NULL;
    return 1;
}

void *VISetPostRetraceCallback(void*);

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

    if(xlHeapTake((void**)&dadd_buf, 0x3000000C)) {
        cpuCompile_DADD_function = dadd_buf;
        dadd_buf[0] = 0x7CA53814; // addc r5, r5, r7
        dadd_buf[1] = 0x7CC64114; // adde r6, r6, r8
        dadd_buf[2] = 0x4E800020; // blr
        DCStoreRange(dadd_buf, 12);
        ICInvalidateRange(dadd_buf, 12);
    }

    if(xlHeapTake((void**)&daddu_buf, 0x3000000C)) {
        cpuCompile_DADDU_function = daddu_buf;
        daddu_buf[0] = 0x7CA53814; // addc r5, r5, r7
        daddu_buf[1] = 0x7CC64114; // adde r6, r6, r8
        daddu_buf[2] = 0x4E800020; // blr
        DCStoreRange(daddu_buf, 12);
        ICInvalidateRange(daddu_buf, 12);
    }

    if(xlHeapTake((void**)&dsub_buf, 0x3000000C)) {
        cpuCompile_DSUB_function = dsub_buf;
        dsub_buf[0] = 0x7CA53814; // adde r5, r7, r6
        dsub_buf[1] = 0x7CC64114; // addc r6, r8, r6
        dsub_buf[2] = 0x4E800020; // blr
        DCStoreRange(dsub_buf, 12);
        ICInvalidateRange(dsub_buf, 12);
    }

    if(xlHeapTake((void**)&dsubu_buf, 0x3000000C)) {
        cpuCompile_DSUBU_function = dsubu_buf;
        dsubu_buf[0] = 0x7CA53814; // adde r5, r7, r6
        dsubu_buf[1] = 0x7CC64114; // addc r6, r8, r6
        dsubu_buf[2] = 0x4E800020; // blr
        DCStoreRange(dsubu_buf, 12);
        ICInvalidateRange(dsubu_buf, 12);
    }

    cpuCompile_S_SQRT(cpu, &cpuCompile_S_SQRT_function);
    cpuCompile_D_SQRT(cpu, &cpuCompile_D_SQRT_function);
    cpuCompile_W_CVT_SD(cpu, &cpuCompile_W_CVT_SD_function);
    cpuCompile_L_CVT_SD(cpu, &cpuCompile_L_CVT_SD_function);
    cpuCompile_CEIL_W(cpu, &cpuCompile_CEIL_W_function);
    cpuCompile_FLOOR_W(cpu, &cpuCompile_FLOOR_W_function);

    if(xlHeapTake((void**)&round_w_buf, 0x3000000C)) {
        cpuCompile_ROUND_W_function = round_w_buf;
        round_w_buf[0] = 0xFC00081C; // fctiw f0, f1
        round_w_buf[1] = 0x7C051FAE; // stfiwx f0, r5, r3
        round_w_buf[2] = 0x4E800020; // blr
        DCStoreRange(round_w_buf, 12);
        ICInvalidateRange(round_w_buf, 12);
    }

    if(xlHeapTake((void**)&trunc_w_buf, 0x3000000C)) {
        cpuCompile_TRUNC_W_function = trunc_w_buf;
        trunc_w_buf[0] = 0xFC00081E; // fctiwz f0, f1
        trunc_w_buf[1] = 0x7C051FAE; // stfiwx f0, r5, r3
        trunc_w_buf[2] = 0x4E800020; // blr
        DCStoreRange(trunc_w_buf, 12);
        ICInvalidateRange(trunc_w_buf, 12);
    }

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
        *entry_p++ = 0x80830000 | ((u32)&cpu->unk_0x34 - (u32)cpu);
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
        *entry_p++ = 0x48000000 | ((u32)entry_node->recompiled_func - (u32)entry_p);
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
   
        }         return 0;

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

int cpuMapObject(cpu_class_t *cpu, void *dev, u32 address_start, u32 address_end, u32 param_5){
    u32 size = address_end - address_start + 1;
    u32 dev_idx;

    if(address_start == 0 && address_end == 0xFFFFFFFF) {
        if(!cpuMakeDevice(cpu, &dev_idx, dev, 0, 0, size, param_5)) {
            return 0;
        }

        cpu->unk_0x1C = dev_idx;
    } else {
        if(!cpuMakeDevice(cpu, &dev_idx, dev, address_start | 0x80000000, address_start, size, param_5)) {
            return 0;
        }

        if(!cpuMakeDevice(cpu, &dev_idx, dev, address_start | 0xA0000000, address_start, size, param_5)) {
            return 0;
        }
    }

    return 1;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8003C708.s")

s32 cpuSetGetBlock(cpu_class_t* cpu, cpu_dev_t* dev, void* arg2) {
    dev->unk_0x2C = arg2;

    return 1;
}

s32 cpuSetDeviceGet(cpu_class_t* cpu, cpu_dev_t* dev, lb_func_t lb, lh_func_t lh, lw_func_t lw, ld_func_t ld)  {
    dev->lb = lb;
    dev->lh = lh;
    dev->lw = lw;
    dev->ld = ld;

    return 1;
}

s32 cpuSetDevicePut(cpu_class_t* cpu, cpu_dev_t* dev, sb_func_t sb, sh_func_t sh, sw_func_t sw, sd_func_t sd)  {
    dev->sb = sb;
    dev->sh = sh;
    dev->sw = sw;
    dev->sd = sd;

    return 1;
}

int func_8003C7E0(cpu_class_t *cpu, u32 addr, u32 repl, s32 end) {
    int i;

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
// Matches except first loop is unrolled twice, and double constant.
inline s32 clearblk(u32 *blk, s32 cnt) {
    s32 i;
    for(i = 0; i < cnt; i++) {
        blk[i] = 0;
    }
}

int cpuReset(cpu_class_t *cpu) {
    int i;
    cpu->unk_0x04 = 0;
    cpu->hack_cnt = 0;
    cpu->status = 0x40;
    cpu->execute_opcode = NULL;

    for(i = 0; i < 0x30; i++) {
        cpu->unk_0x248[i].unk_0x00.d = 0;
        cpu->unk_0x248[i].unk_0x08.d = 0;
        cpu->unk_0x248[i].unk_0x10.d = 0;
        cpu->unk_0x248[i].unk_0x18.d = 0;
        cpu->unk_0x248[i].unk_0x20.d = -1;
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

    func_8000E054(cpu, 0x20010000, 0, 0x2000FF01, 1);

    cpu->cp0[0x10].d = 0x6E463;
    cpu->unk_0x18 = 0;

    if(func_8000CB1C(cpu)) {
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
int cpuReset(cpu_class_t *cpu);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuReset.s")
#endif

int cpuGetXPC(cpu_class_t *cpu, u64 *param_2, u64 *param_3, u64 *param_4) {
    if(!xlObjectTest(cpu, &lbl_80171F38)) {
        return 0;
    }

    if(param_2 != NULL) {
        *param_2 = cpu->pc;
    }

    if(param_3 != NULL) {
        *param_3 = cpu->lo.d;
    }

    if(param_4 != NULL) {
        *param_4 = cpu->hi.d;
    }

    return 1;
}

int cpuSetXPC(cpu_class_t *cpu, u32 r4, u64 pc, u64 lo, u64 hi) {
    if(!xlObjectTest(cpu, &lbl_80171F38)) {
        return 0;
    }
    
    cpu->pc = pc;
    cpu->status |= 4;
    cpu->lo.d = lo;
    cpu->hi.d = hi;

    return 1;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuEvent.s")

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

int cpuHeapTake(char **code, cpu_class_t *cpu, recomp_node_t *func, int size);

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

#ifdef NON_MATCHING
// swapped operands in and. and add instructions
s32 cpuTreeTake(recomp_node_t **node, u32 *node_pos, size_t size) {
    s32 node_found = 0;
    s32 i;
    s32 j;
    u32 *node_status = gSystem->cpu->tree_blk_status;

    for(i = 0; i < 125; i++) {
        if(node_status[i] != 0xFFFFFFFF) {
            u32 mask = (1 << 1) - 1;

            j = 32;
            do {
                if(!(node_status[i] & mask)) {
                    node_found = 1;
                    node_status[i] |= mask;
                    *node_pos = (i * 32) + (32 - j);
                    *node_pos |= 0x10000;
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
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuTreeTake.s")
#endif

extern u8 lbl_801709C0[64];
extern u8 lbl_80170A00[32];
extern u8 lbl_80170980[64];
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuFindFunction.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8003E604.s")