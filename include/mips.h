#ifndef _MIPS_H
#define _MIPS_H

#define MIPS_OP(inst) ((inst >> 0x1A) & 0x3F)

#define OPC_SPECIAL 0
#define OPC_REGIMM 1
#define OPC_J 2
#define OPC_JAL 3
#define OPC_BEQ 4
#define OPC_BNE 5
#define OPC_BLEZ 6
#define OPC_BGTZ 7
#define OPC_ADDI 8
#define OPC_ADDIU 9
#define OPC_SLTI 10
#define OPC_SLTIU 11
#define OPC_ANDI 12
#define OPC_ORI 13
#define OPC_XORI 14
#define OPC_LUI 15
#define OPC_CP0 16
#define OPC_CP1 17

#define SPEC_FUNCT(inst) ((inst >> 0) & 0x3F)
#define SPEC_SLL 0
#define SPEC_SRL 2
#define SPEC_SRA 3
#define SPEC_SLLV 4
#define SPEC_SRLV 6
#define SPEC_SRAV 7
#define SPEC_JR 8
#define SPEC_JALR 9
#define SPEC_SYSCALL 12
#define SPEC_BREAK 13
#define SPEC_MFHI 16
#define SPEC_MTHI 17
#define SPEC_MFLO 18
#define SPEC_MTLO 19
#define SPEC_DSLLV 20
#define SPEC_DSRLV 22
#define SPEC_DSRAV 23
#define SPEC_MULT 24
#define SPEC_MULTU 25
#define SPEC_DIV 26
#define SPEC_DIVU 27
#define SPEC_DMULT 28
#define SPEC_DMULTU 29
#define SPEC_DDIV 30
#define SPEC_DDIVU 31
#define SPEC_ADD 32
#define SPEC_ADDU 33
#define SPEC_SUB 34
#define SPEC_SUBU 35
#define SPEC_AND 36
#define SPEC_OR 37
#define SPEC_XOR 38
#define SPEC_NOR 39
#define SPEC_SLT 42
#define SPEC_SLTU 43
#define SPEC_DADD 44
#define SPEC_DADDU 45
#define SPEC_DSUB 46
#define SPEC_DSUBU 47
#define SPEC_TGE 48
#define SPEC_TGEU 49
#define SPEC_TLT 50
#define SPEC_TLTU 51 
#define SPEC_TEQ 52
#define SPEC_TNE 54
#define SPEC_DSLL 56
#define SPEC_DSRL 58
#define SPEC_DSRA 59
#define SPEC_DSLL32 60
#define SPEC_DSRL32 62
#define SPEC_DSRA32 63

#define REGIMM_SUB(inst) ((inst >> 16) & 0x1F)
#define REGIMM_BLTZ 0
#define REGIMM_BGEZ 1
#define REGIMM_BLTZL 2
#define REGIMM_BGEZL 3
#define REGIMM_TGEI 8
#define REGIMM_TGEIU 9
#define REGIMM_TLTI 10
#define REGIMM_TLTIU 11
#define REGIMM_TEQI 12
#define REGIMM_TNEI 14
#define REGIMM_BLTZAL 16
#define REGIMM_BGEZAL 17
#define REGIMM_BLTZALL 18
#define REGIMM_BGEZALL 19

#define MIPS_RD(inst) ((inst >> 0xB) & 0x1F)
#define MIPS_RT(inst) ((inst >> 0x10) & 0x1F)
#define MIPS_RS(inst) ((inst >> 0x15) & 0x1F)

#define MIPS_FDT(inst) ((inst >> 0) & 0x7FF)
#define MIPS_FR(inst) ((inst >> 0x15) & 0x1F)
#define MIPS_FT(inst) ((inst >> 0x10) & 0x1F)
#define MIPS_FS(inst) ((inst >> 0xB) & 0x1F)
#define MIPS_FD(inst) ((inst >> 6) & 0x1F)
#define MIPS_FSUB(inst) ((inst >> 0x15) & 0x1F)
#define MIPS_FFMT(inst) ((inst >> 0x15) & 0x1F)
#define MIPS_FFUNC(inst) ((inst >> 0x0) & 0x3F)
#define MIPS_FCC(inst) ((inst >> 8) & 7)
#define MIPS_FFMT3(inst) ((inst >> 0) & 7)
#define MIPS_FOP4(inst) ((inst >> 3) & 7)

#define MIPS_FMT_SINGLE 16
#define MIPS_FMT_DOUBLE 17
#define MIPS_FMT_WORD 20
#define MIPS_FMT_DBLWORD 21

#define MIPS_FSUB_MFC 0
#define MIPS_FSUB_DMFC 1
#define MIPS_FSUB_CFC 2
#define MIPS_FSUB_MTC 4
#define MIPS_FSUB_DMTC 5
#define MIPS_FSUB_CTC 6

#define MIPS_IMM(inst) ((s16)(inst & 0xFFFF))
#define MIPS_SA(inst ) ((inst >> 6) & 0x1F)
#endif