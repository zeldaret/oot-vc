#include "cpu.h"
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

extern class_t lbl_80171F38;
extern u32 lbl_80170B68[32];
extern void (*lbl_8025CFE8)();

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuTestInterrupt.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuException.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuMakeDevice.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000D580.s")

s32 func_8000D6EC(cpu_class_t *cpu, s32 arg1);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000D6EC.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000DEBC.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000DF58.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E054.s")

s32 func_8000E0E8(cpu_class_t *cpu, s32 reg, u32 hi, u32 lo);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E0E8.s")

s32 func_8000E2B0(cpu_class_t *cpu, s32, u32*);
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E2B0.s")

int func_8000E43C(cpu_class_t *cpu) {
    if(cpu->cp0[0xC].d & 4) {
        cpu->pc = cpu->cp0[0x1E].d;
        cpu->cp0[0xC].d &= ~4;
    } else {
        cpu->pc = cpu->cp0[0xE].d;
        cpu->cp0[0xC].d &= ~2;
    }

    cpu->unk_0x00 |= 0x24;
    return 1;
}

int __cpuBreak(cpu_class_t *cpu) {
    cpu->unk_0x00 |= 2;
    return 1;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E4B8.s")

#ifdef NON_MATCHING
// matches, but jump table
int cpuCheckDelaySlot(u32 inst) {
    s32 t;
    int ret = 0;
    if(inst == 0) {
        return 0;
    }

    switch(inst >> 0x1A) {
        case 0:
            switch(inst & 0x3F) {
                case 8:
                    ret = 0xD05;
                    break;
                case 9:
                    ret = 0x8AE;
                    break;
            }
            break;
        case 1:
            switch((inst >> 0x10) & 0x1F) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                    ret = 0x457;
                    break;
            }
            break;
        case 0x10:
            switch(inst & 0x3F) {
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
        case 0x11:
            if(((inst >> 0x15) & 0x1F) == 8) {
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
        case 2:
            ret = 0xD05;
            break;
        case 3:
            ret = 0x8AE;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
            ret = 0x457;
            break;
    }

    return ret;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuCheckDelaySlot.s")
#endif

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E654.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E734.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8000E81C.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuGetPPC.s")

int func_80031D4C() {
    return 0;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuMakeFunction.s")

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
void *cpuExecuteOpcode(cpu_class_t *cpu, s32 arg1, u32 addr, u32 *ret) {
    u32 tick = OSGetTick();
    u32 *code_buf;
    u32 inst;
    reg64_t prev_ra;

    if(cpu->unk_0x24 != 0) {
        cpu->unk_0x00 |= 8;
    } else {
        cpu->unk_0x00 &= ~8;
    }

    if(gSystem->rom_id == 'NKTJ') {
        if(addr == 0x802A4118) {
            gSystem->frame[0x11] = 0;
        }
        if(addr == 0x80072D94) {
            gSystem->frame[0x11] = 1;
        }
    } else if(gSystem->rom_id == 'NKTP') {
        if(addr == 0x802A4160) {
            gSystem->frame[0x11] = 0;
        }
        if(addr == 0x80072E34) {
            gSystem->frame[0x11] = 1;
        }
    } else if(gSystem->rom_id == 'NKTE') {
        if(addr == 0x802A4160) {
            gSystem->frame[0x11] = 0;
        }
        if(addr == 0x80072E54) {
            gSystem->frame[0x11] = 1;
        }
    }

    if(!cpuGetAddressBuffer(cpu, (void**)&code_buf, addr)) {
        return NULL;
    }

    inst = *code_buf;
    cpu->pc = addr + 4;
    if(inst == 0xACBF011C) {
        prev_ra = cpu->gpr[0x1F];
        cpu->gpr[0x1F].w[1] = cpu->unk_0x30;
    }

    switch(inst >> 0x1A) {
        case 0:
            switch(inst & 0x3F) {
                case 0: // SLL
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] << ((inst >> 6) & 0x1F);
                    break;
                case 2: // SRL
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] >> ((inst >> 6) & 0x1F);
                    break;
                case 3: // SRA
                    cpu->gpr[(inst >> 0xB) & 0x1F].sw[1] = cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] >> ((inst >> 6) & 0x1F);
                    break;
                case 4: // SLLV
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] << (cpu->gpr[(inst >> 0x15) & 0x1F].w[1] & 0x1F);
                    break;
                case 6: // SRLV
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = cpu->gpr[(inst >> 0x10) & 0x1F].w[1] >> (cpu->gpr[(inst >> 0x15) & 0x1F].w[1] & 0x1F);
                    break;
                case 7: // SRAV
                    cpu->gpr[(inst >> 0xB) & 0x1F].sw[1] = cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] >> (cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] & 0x1F);
                    break;
                case 8: // JR
                    cpu->unk_0x24 = cpu->gpr[(inst >> 0x15) & 0x1F].w[1];
                    break;
                case 9: // JALR
                    cpu->unk_0x24 = cpu->gpr[(inst >> 0x15) & 0x1F].w[1];
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->pc + 4;
                    break;
                case 10:
                case 11:
                    break;
                case 12: // SYSCALL
                    cpuException(cpu, 8, 0);
                    break;
                case 13: // BREAk
                    cpuException(cpu, 9, 0);
                    break;
                case 14:
                case 15:
                    break;
                case 16: // MFHI
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->hi.d;
                    break;
                case 17: // MTHI
                    cpu->hi.d = cpu->gpr[(inst >> 0x15) & 0x1F].d;
                    break;
                case 18: // MFLO
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->lo.d;
                    break;
                case 19: // MTLO
                    cpu->lo.d = cpu->gpr[(inst >> 0xB) & 0x1F].d;
                    break;
                case 20: // DSLLV
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->gpr[(inst >> 0x10) & 0x1F].d << (cpu->gpr[(inst >> 0x15) & 0x1F].d & 0x1F);
                    break;
                case 21:
                    break;
                case 22: // DSRLV
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->gpr[(inst >> 0x10) & 0x1F].d >> (cpu->gpr[(inst >> 0x15) & 0x1F].d & 0x1F);
                    break;
                case 23: // DRAV
                    cpu->gpr[(inst >> 0xB) & 0x1F].sd = cpu->gpr[(inst >> 0x10) & 0x1F].sd >> (cpu->gpr[(inst >> 0x15) & 0x1F].sd & 0x1F);
                    break;
                case 24: // MULT
                    {
                        reg64_t r;
                        r.sd = (s64)(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] * cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]);
                        cpu->lo.sd = r.w[0];
                        cpu->hi.sd = r.w[1];
                    }
                    break;
                case 25: // MULTU
                    {
                        reg64_t r;
                        u64 rs = cpu->gpr[(inst >> 0x15) & 0x1F].w[1];
                        u64 rt = cpu->gpr[(inst >> 0x10) & 0x1F].w[1];
                        r.sd = rs * rt;
                        cpu->lo.sd = r.d & 0xFFFFFFFF;
                        cpu->hi.d  = r.w[1] >> 32;
                    }
                    break;
                case 26: // DIV
                    {
                        u32 res;
                        if(cpu->gpr[(inst >> 0x10) & 0x1F].w[1] == 0) {
                            break;
                        }

                        res = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] / cpu->gpr[(inst >> 0x10) & 0x1F].sw[1];
                        cpu->lo.d = res;

                        res = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] % cpu->gpr[(inst >> 0x10) & 0x1F].sw[1];
                        cpu->hi.d = res;
                    }
                    break;
                case 27: // DIVU
                    {
                        u64 res;
                        if(cpu->gpr[(inst >> 0x10) & 0x1F].w[1] == 0) {
                            break;
                        }

                        cpu->lo.d = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] / cpu->gpr[(inst >> 0x10) & 0x1F].w[1];
                        cpu->hi.d = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] % cpu->gpr[(inst >> 0x10) & 0x1F].w[1];
                    }
                    break;

                case 28: // DMULT
                {
                    s64 prod = cpu->gpr[(inst >> 0x15) & 0x1F].sd * cpu->gpr[(inst >> 0x10) & 0x1F].sd;
                    cpu->lo.sd = prod;
                    cpu->hi.sd = prod >> 32;
                }
                break;
                case 29: // DMULTU
                {
                    u64 prod = cpu->gpr[(inst >> 0x15) & 0x1F].d * cpu->gpr[(inst >> 0x10) & 0x1F].d;
                    cpu->lo.d = prod;
                    cpu->hi.d = prod >> 32;
                }
                break;
                case 30: // DDIV
                    if(cpu->gpr[(inst >> 0x10) & 0x1F].w[1] == 0) {
                        break;
                    }

                    cpu->lo.sd = cpu->gpr[(inst >> 0x15) & 0x1F].sd / cpu->gpr[(inst >> 0x10) & 0x1F].sd;
                    cpu->hi.sd = cpu->gpr[(inst >> 0x15) & 0x1F].sd % cpu->gpr[(inst >> 0x10) & 0x1F].sd;
                    break;

                case 31: // DDIVU
                    if(cpu->gpr[(inst >> 0x10) & 0x1F].w[1] == 0) {
                        break;
                    }

                    cpu->lo.d = cpu->gpr[(inst >> 0x15) & 0x1F].d / cpu->gpr[(inst >> 0x10) & 0x1F].d;
                    cpu->hi.d = cpu->gpr[(inst >> 0x15) & 0x1F].d % cpu->gpr[(inst >> 0x10) & 0x1F].d;
                    break;
                case 32: // ADD
                    cpu->gpr[(inst >> 0xB) & 0x1F].sw[1] = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + cpu->gpr[(inst >> 0x10) & 0x1F].sw[1];
                    break;
                case 33: // ADDU
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + cpu->gpr[(inst >> 0x10) & 0x1F].w[1];
                    break;
                case 34:  // SUB
                    cpu->gpr[(inst >> 0xB) & 0x1F].sw[1] = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] - cpu->gpr[(inst >> 0x10) & 0x1F].sw[1];
                    break;
                case 35: // SUBU
                    cpu->gpr[(inst >> 0xB) & 0x1F].sw[1] = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] - cpu->gpr[(inst >> 0x10) & 0x1F].sw[1];
                    break;
                case 36: // AND
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] & cpu->gpr[(inst >> 0x10) & 0x1F].w[1];
                    break;
                case 37: // AND
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] | cpu->gpr[(inst >> 0x10) & 0x1F].w[1];
                    break;
                case 38: // XOR
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] ^ cpu->gpr[(inst >> 0x10) & 0x1F].w[1];
                    break;
                case 39: // NOR
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = ~(cpu->gpr[(inst >> 0x15) & 0x1F].w[1] | cpu->gpr[(inst >> 0x10) & 0x1F].w[1]);
                    break;
                case 40:
                case 41:
                    break;
                case 42: // SLT
                    cpu->gpr[(inst >> 0xB) & 0x1F].sw[1] = !!(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]);
                    break;
                case 43: // SLTU
                    cpu->gpr[(inst >> 0xB) & 0x1F].w[1] = !!(cpu->gpr[(inst >> 0x15) & 0x1F].w[1] < cpu->gpr[(inst >> 0x10) & 0x1F].w[1]);
                    break;
                case 44: // DADD
                    cpu->gpr[(inst >> 0xB) & 0x1F].sd = cpu->gpr[(inst >> 0x15) & 0x1F].sd + cpu->gpr[(inst >> 0x10) & 0x1F].sd;
                    break;
                case 45: // DADDU
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->gpr[(inst >> 0x15) & 0x1F].d + cpu->gpr[(inst >> 0x10) & 0x1F].d;
                    break;
                case 46: // DSUB
                    cpu->gpr[(inst >> 0xB) & 0x1F].sd = cpu->gpr[(inst >> 0x15) & 0x1F].sd - cpu->gpr[(inst >> 0x10) & 0x1F].sd;
                    break;
                case 47: // DSUBU
                    cpu->gpr[(inst >> 0xB) & 0x1F].sd = cpu->gpr[(inst >> 0x15) & 0x1F].sd - cpu->gpr[(inst >> 0x10) & 0x1F].sd;
                    break;
                case 48: // TGE
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] >= cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 49: // TGEU
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].w[1] >= cpu->gpr[(inst >> 0x10) & 0x1F].w[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 50: // TGE
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 51: // TGEU
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].w[1] < cpu->gpr[(inst >> 0x10) & 0x1F].w[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 52: // TEQ
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] == cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 53:
                    break;
                case 54: // TNE
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] != cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 55:
                    break;
                case 56: // DSLL
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->gpr[(inst >> 0x10) & 0x1F].d << ((inst >> 6) & 0x1F);
                    break;
                case 57: 
                    break;
                case 58: // DSRL
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->gpr[(inst >> 0x10) & 0x1F].d >> ((inst >> 6) & 0x1F);
                    break;
                case 59: // DSRA
                    cpu->gpr[(inst >> 0xB) & 0x1F].sd = cpu->gpr[(inst >> 0x10) & 0x1F].sd >> ((inst >> 6) & 0x1F);
                    break;
                case 60: // DSLL32
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->gpr[(inst >> 0x10) & 0x1F].d << (((inst >> 6) & 0x1F) + 32);
                    break;
                case 61:
                    break;
                case 62: // DSRL32
                    cpu->gpr[(inst >> 0xB) & 0x1F].d = cpu->gpr[(inst >> 0x10) & 0x1F].d >> (((inst >> 6) & 0x1F) + 32);
                    break;
                case 63: // DSRA32
                    cpu->gpr[(inst >> 0xB) & 0x1F].sd = cpu->gpr[(inst >> 0x10) & 0x1F].sd >> (((inst >> 6) & 0x1F) + 32);
                    break;

            }       
            break;
        case 1:
            switch((inst >> 0x10) & 0x1F) {
                case 0: // BLTZ
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < 0) {
                        cpu->unk_0x24 = cpu->pc + ((s16)(inst & 0xFFFF) * 4);
                    }
                    break;
                case 1: // BGEZ
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] >= 0) {
                        cpu->unk_0x24 = cpu->pc + ((s16)(inst & 0xFFFF) * 4);
                    }
                    break;
                case 2: // BLTZL
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < 0) {
                        cpu->unk_0x24 = cpu->pc + ((s16)(inst & 0xFFFF) * 4);
                    } else {
                        cpu->unk_0x00 |= 4;
                        cpu->pc += 4;
                    }
                    break;
                case 3: // BGEZL
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] >= 0) {
                        cpu->unk_0x24 = cpu->pc + ((s16)(inst & 0xFFFF) * 4);
                    } else {
                        cpu->unk_0x00 |= 4;
                        cpu->pc += 4;
                    }
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    break;
                case 8: // TGEI
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] >= (s16)(inst & 0xFFFF)) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 9: // TGEIU
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].w[1] >= (s16)(inst & 0xFFFF)) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 10: // TLTI
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < (s16)(inst & 0xFFFF)) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 11: // TLTIU
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].w[1] < (s16)(inst & 0xFFFF)) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 12: // TEQI
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] == (s16)(inst & 0xFFFF)) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 13:
                    break;
                case 14: // TNEI
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] != (s16)(inst & 0xFFFF)) {
                        cpuException(cpu, 0xD, 0);
                    }
                    break;
                case 15:
                    break;
                case 16: // BLTZAL
                    cpu->gpr[0x1F].w[1] = cpu->pc + 4;
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < 0) {
                        cpu->unk_0x24 = cpu->unk_0x28 = cpu->pc + ((s16)(inst & 0xFFFF) * 4);
                    }
                    break;
                case 17: // BGEZAL
                    cpu->gpr[0x1F].w[1] = cpu->pc + 4;
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] >= 0) {
                        cpu->unk_0x24 = cpu->unk_0x28 = cpu->pc + ((s16)(inst & 0xFFFF) * 4);
                    }
                    break;
                case 18: // BLTZALL
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < 0) {
                        cpu->gpr[0x1F].sw[1] = cpu->pc + 4;
                        cpu->unk_0x24 = cpu->pc + ((s16)(inst & 0xFFFF) * 4);
                    } else {
                        cpu->unk_0x00 |= 4;
                        cpu->pc += 4;
                    }
                    break;
                case 19: // BGEZALL
                    if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] >= 0) {
                        cpu->gpr[0x1F].sw[1] = cpu->pc + 4;
                        cpu->unk_0x24 = cpu->pc + ((s16)(inst & 0xFFFF) * 4);
                    } else {
                        cpu->unk_0x00 |= 4;
                        cpu->pc += 4;
                    }
                    break;
            }
            break;
        case 2: // J
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
        case 3: //JAL
            {
                recomp_node_t *node;
    
                cpu->gpr[0x1F].w[1] = cpu->pc + 4;
                cpu->unk_0x24 = cpu->unk_0x28 = (cpu->pc & 0xF0000000) | ((inst & 0x3FFFFFF) << 2);
                cpuFindFunction(cpu, cpu->unk_0x24, &node);
            }
            break;
        case 4: // BEQ
             if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] == cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                 cpu->unk_0x24 = cpu->pc + (s16)(inst & 0xFFFF) * 4;
             }

             if(cpu->unk_0x24 != cpu->pc - 4) {
                 break;
             }

             if(!checkRetrace()) {
                 return NULL;
             }
             break;
        case 5: // BNE
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] != cpu->gpr[(inst >> 0x10) & 0x1F].sw[1]) {
                cpu->unk_0x24 = cpu->pc + (s16)(inst & 0xFFFF) * 4;
            }
            break;
        case 6: // BLEZ
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] <= 0) {
                cpu->unk_0x24 = cpu->pc + (s16)(inst & 0xFFFF) * 4;
            }
            break;
        case 7: // BGTZ
            if(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] > 0) {
                cpu->unk_0x24 = cpu->pc + (s16)(inst & 0xFFFF) * 4;
            }
            break;
        case 8: // ADDI
            cpu->gpr[(inst >> 0x10) & 0x1F].sw[1] = cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] + (s16)(inst & 0xFFFF);
            break;
        case 9: // ADDIU
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] + (s16)(inst & 0xFFFF);
            break;
        case 10: // SLTI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = !!(cpu->gpr[(inst >> 0x15) & 0x1F].sw[1] < (s16)(inst & 0xFFFF));
            break;
        case 11: // SLTIU
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = !!(cpu->gpr[(inst >> 0x15) & 0x1F].w[1] < (s16)(inst & 0xFFFF));
            break;
        case 12: // ANDI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] & (inst & 0xFFFF);
            break;
        case 13: // ORI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] | (inst & 0xFFFF);
            break;
        case 14: // XORI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = cpu->gpr[(inst >> 0x15) & 0x1F].w[1] ^ (inst & 0xFFFF);
            break; 
        case 15: // LUI
            cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = (inst & 0xFFFF) << 0x10;
            break;
        case 16: // CP0
            switch(inst & 0x3F) {
                case 1: // TLBR
                    cpu->cp0[2].d = cpu->unk_0x248[cpu->cp0[0].w[1] & 0x3F].unk_0x00.d;
                    cpu->cp0[3].d = cpu->unk_0x248[cpu->cp0[0].w[1] & 0x3F].unk_0x08.d;
                    cpu->cp0[0xA].d = cpu->unk_0x248[cpu->cp0[0].w[1] & 0x3F].unk_0x10.d;
                    cpu->cp0[5].d = cpu->unk_0x248[cpu->cp0[0].w[1] & 0x3F].unk_0x18.d;
                    break;
                case 2: // TLBWI
                    func_8000D6EC(cpu, cpu->cp0[0].w[1] & 0x3F);
                    break;
                case 5: // TLBWR
                    {
                        s32 i;
                        u32 val;

                        for(i = 0; i < 0x30; i++) {
                            if(!(cpu->unk_0x248[i].unk_0x10.w[1] & 2)) {
                                val++;
                            }
                        }

                        cpu->cp0[1].d = val;
                        func_8000D6EC(cpu, val);
                    }
                    break;
                case 8: // TLBP
                    {
                        s32 i;
                        u32 val;

                        cpu->cp0[0].w[1] |= 0x80000000;

                        for(i = 0; i < 0x30; i++) {
                            if(cpu->unk_0x248[i].unk_0x00.w[1] & 2) {
                                if(!(cpu->cp0[10].d ^ cpu->unk_0x248[i].unk_0x10.d)) {
                                    cpu->cp0[0].d = val;
                                }
                            }
                        }

                        break;
                    }
                case 24: // ERET
                    if(cpu->cp0[12].d & 4) {
                        cpu->pc = cpu->cp0[30].w[1];
                        cpu->cp0[12].d &= ~4;
                    } else {
                        cpu->pc = cpu->cp0[14].w[1];
                        cpu->cp0[12].d &= ~2;
                    }
                    cpu->unk_0x00 |= 0x24;
                    break;
                case 0: // SUB
                case 3:
                case 4: 
                case 6:
                case 7:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 18:
                case 19:
                case 20:
                case 21:
                case 22:
                case 23:
                    switch((inst >> 0x15) & 0x1F) {
                        case 0: // MFC0
                            {
                                u32 res;
                            
                                if(!func_8000E2B0(cpu, (inst >> 0xB) & 0x1F, &res)) {
                                    break;
                                }

                                cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = res;
                            }
                            break;
                        case 1: // DMFC0
                            {
                                u32 res[2];
                                if(!func_8000E2B0(cpu, (inst >> 0xB) & 0x1F, res)) {
                                    break;
                                }

                                cpu->gpr[(inst >> 0x10) & 0x1F].w[1] = res[1];
                                cpu->gpr[(inst >> 0x10) & 0x1F].w[0] = res[0];
                            }
                            break;
                        case 2:
                        case 3:
                            break;
                        case 4: // MTC0
                            func_8000E0E8(cpu, (inst >> 0xB) & 0x1F, 0, cpu->gpr[(inst >> 0x10) & 0x1F].w[1]);
                            break;
                        case 5: // DMTC0
                            func_8000E0E8(cpu, (inst >> 0xB) & 0x1F, cpu->gpr[(inst >> 0x10) & 0x1F].w[0], cpu->gpr[(inst >> 0x10) & 0x1F].w[1]);
                            break;
                        case 6:
                        case 7:
                        case 8:
                            break;
                    }
                    break;


            }
    }

    if(!cpuExecuteUpdate(cpu, &ret, tick + 1)) {
        return NULL;
    }

    if(inst == 0xACBF011C) {
        cpu->gpr[0x1F] = prev_ra;
    }
}
//#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteOpcode.s")

s32 videoForceRetrace(void *);
s32 cpuExecuteUpdate(cpu_class_t *cpu, u32 **, u32);

#ifdef NON_MATCHING
void *cpuExecuteIdle(cpu_class_t *cpu, u32 r4, u32 addr, u32 *ret) {
    rom_class_t *rom;
    u32 tick;

    rom = gSystem->rom;
    tick = OSGetTick();

    if(cpu->unk_0x24 != 0) {
        cpu->unk_0x00 |= 8;
    } else {
        cpu->unk_0x00 &= ~8;
    }

    cpu->pc = addr;
    cpu->unk_0x00 |= 0x80;

    if(!(cpu->unk_0x00 & 0x40) && !rom->unk_19A34) {
        videoForceRetrace(gSystem->video);
    }

    if(!cpuExecuteUpdate(cpu, &ret, tick)) {
        return NULL;
    }

    cpu->unk_0x38 = OSGetTick();
    
    return ret;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteIdle.s")
#endif

void *cpuExecuteJump(cpu_class_t *cpu, u32 r4, u32 addr, u32 *ret) {
    u32 tick = OSGetTick();

    if(cpu->unk_0x24 != 0) {
        cpu->unk_0x00 |= 8;
    } else {
        cpu->unk_0x00 &= ~8;
    }

    cpu->pc = addr;
    cpu->unk_0x00 |= 4;
    if(!cpuExecuteUpdate(cpu, &ret, tick)) {
        return NULL;
    }

    cpu->unk_0x38 = OSGetTick();

    return ret;
}

#ifdef NON_MATCHING
inline void tf(cpu_class_t *cpu, s32 addr, u32 *ret) {
    s32 i;
    recomp_node_t *node;
    unk_node_0x18 *unk_0x18;
    u32 *ret_p = ret - 1;
    
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

void *cpuExecuteCall(cpu_class_t *cpu, u32 arg1, u32 addr, u32 *ret) {
    u32 tick = OSGetTick();
    recomp_node_t *node;
    s32 i;
    s32 flg;
    u32 *ret_p;
    u32 *ret_p2;

    if(cpu->unk_0x24 != 0) {
        cpu->unk_0x00 |= 8;
    } else {
        cpu->unk_0x00 &= ~8;
    }

    cpu->unk_0x00 |= 4;
    cpu->pc = addr;
    cpu->gpr[0x1F].w[1] = (u32)ret;
    cpu->unk_0x34++;

    tf(cpu, addr, ret);

    flg = ((lbl_80170B68[31] >> 8) & 1) != 0;
    ret_p = ret - (!!flg + 3);
    if(flg) {
        ret_p[0] = ((u32)ret >> 0x10) | 0x3CA00000;
        ret_p[1] = ((u32)ret & 0xFFFF) | 0x60A50000;
        DCStoreRange(ret_p, 8);
        ICInvalidateRange(ret_p, 8);
    } else {
        u32 rs = lbl_80170B68[31] << 0x15;
        u32 ra = lbl_80170B68[31] << 0x10;
        ret_p[0] = 0x3C000000 | rs | ((u32)ret >> 0x10);
        ret_p[1] = 0x60000000 | rs | ra | ((u32)ret & 0xFFFF);
        DCStoreRange(ret_p, 8);
        ICInvalidateRange(ret_p, 8);
    }

    if(!cpuExecuteUpdate(cpu, &ret, tick)) {
        return NULL;
    }

    if(flg) {
        ret_p[3] = 0x48000000 | (((u32)ret - (u32)&ret_p[3]) & 0x3FFFFFC);
        DCStoreRange(ret_p, 16);
        ICInvalidateRange(ret_p, 16);
    } else {
        ret_p[2] = 0x48000000 | (((u32)ret - (u32)&ret_p[3]) & 0x3FFFFFC);
        DCStoreRange(ret_p, 12);
        ICInvalidateRange(ret_p, 12);
    }

    cpu->unk_0x38 = OSGetTick();

    return ret;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteCall.s")
#endif

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteLoadStore.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecuteLoadStoreF.s")

int cpuMakeLink(cpu_class_t *cpu, u32 **link_code, void *handler) {
    u32 *code;
    s32 i;
    u32 paddr;
    s32 cnt = sizeof(lbl_80170B68) / sizeof(*lbl_80170B68);
    
    if(!xlHeapTake((void**)&code, 0x30000200)) {
        return 0;
    }

    *link_code = code;
    *code++ = 0x7CC802A6; // mflr r6

    for(i = 1; i < 32; i++) {
        if(!(lbl_80170B68[i] & 0x100)) {
            *code++ = (((u32)&cpu->gpr[i] - (u32)cpu) + 4) |
                      (lbl_80170B68[i] << 0x15) |
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

    *code++ = (lbl_80170B68[0] << 0x15) |
              0x38000000;                                       // li r10, 0

    for(i = 1; i < 32; i++) {
        if(!(lbl_80170B68[i] & 0x100)) {
            *code++ = (((u32)&cpu->gpr[i] - (u32)cpu) + 4) |
                      (lbl_80170B68[i] << 0x15) |
                      0x80030000;                               // lwz rx, off(r3)
        }
    }

    *code++ = 0x4E800020;                                       // blr

    DCStoreRange(*link_code, 0x200);
    ICInvalidateRange(*link_code, 0x200);

    return 1;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuExecute.s")

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
    cpu->unk_0x00 = 0x40;
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
        cpu->unk_0x00 |= 0x10;
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
    cpu->unk_0x00 |= 4;
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

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/cpuFindFunction.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/cpu/func_8003E604.s")